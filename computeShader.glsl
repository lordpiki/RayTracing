#version 430 core
layout (local_size_x = 16, local_size_y = 16) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (rgba32f, binding = 1) uniform image2D imgAccumulation;

#define RED vec3(1,0,0);
#define GREEN vec3(0,1,0);
#define BLUE vec3(0,0,1);
#define WHITE vec3(1,1,1);
#define BLACK vec3(0,0,0);
#define PURPLE vec3(1,0,1);


struct Material {
    vec4 color;
    vec3 emission;
    float emissionStrength;
};



struct MeshInfo {
    vec3 boundsMin;
    uint triangleCount;
    Material material;
    vec3 boundsMax;
    uint triangleIndex;
};


struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

struct Triangle {
	vec3 posA;
	float padd1;
	vec3 posB;
	float padd2;
	vec3 posC;
	float padd3;
    //vec3 normalA, normalB, normalC;
    };

struct Ray {
	vec3 origin;
	vec3 dir;
};

struct HitInfo
{
    bool hit;
    float dst;
    vec3 point;
    vec3 normal;
    Material material;
};

struct TriangleHitInfo {
	bool hit;
	float dst;
	vec3 point;
	vec3 normal;
	int triIndex;
};


layout(std430, binding = 0) buffer SphereBuffer {
    Sphere spheres[];
};


layout (std430, binding = 1) buffer MeshBuffer {
	MeshInfo meshes[];
};

layout (std140, binding = 2) buffer TriangleBuffer {
	Triangle triangles[];
};

// camera 
uniform vec3 center;
uniform vec3 pixel00_loc;
uniform vec3 pixel_delta_u;
uniform vec3 pixel_delta_v;
ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

// else
uniform float randomSeed;
uniform int maxDepth;
uniform int raysPerPixel;
uniform int frameNum = 0;
int seed = 1;

#define PI 3.14159265359 


HitInfo hit_sphere(vec3 center, Ray ray, Sphere sphere)
{

    vec3 oc = center - ray.origin;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b*b - 4*a*c;

    HitInfo hitInfo;
    hitInfo.hit = false;
    hitInfo.dst = 9e9;

    if (discriminant < 0)
    {
        return hitInfo;
    }
    
    float dst = (-b - sqrt(discriminant)) / (2.0f * a);

    // Check if the sphere is behind or in front of the ray
    if (dst < 0)
    {
        return hitInfo;
    }

    hitInfo.hit = true;
    hitInfo.dst = dst;
    hitInfo.point = ray.origin + dst * ray.dir;
    hitInfo.normal = (hitInfo.point - center) / sphere.radius;
    hitInfo.material = sphere.material;

    return hitInfo;
}


bool hit_mesh(Ray ray, vec3 boundsMin, vec3 boundsMax)
{

    const float EPSILON = 1e-6;
    vec3 invDir = 1.0 / (ray.dir + vec3(EPSILON));
    vec3 t0 = (boundsMin - ray.origin) * invDir;
    vec3 t1 = (boundsMax - ray.origin) * invDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    float dst = tNear;
    vec3 hitPoint = ray.origin + ray.dir * dst;
    vec3 normal = -sign(ray.dir) * step(tmin.yzx, tmin.xyz) * step(tmin.zxy, tmin.xyz);
    
    return tNear <= tFar && tFar > EPSILON;
}

TriangleHitInfo hit_triangle(Ray ray, Triangle tri)
{
	vec3 edge1 = tri.posB - tri.posA;
	vec3 edge2 = tri.posC - tri.posA;
    vec3 normalVec = cross(edge1, edge2);
    vec3 ao = ray.origin - tri.posA;
    vec3 dao = cross(ao, ray.dir);

    

    float determ = -dot(ray.dir, normalVec);
    float invDet = 1 / determ;

    // Calculate dst to triangle & barycentric coordinates of intersection point
    float dst = dot(ao, normalVec) * invDet;
    float u = dot(edge2, dao) * invDet;
    float v = -dot(edge1, dao) * invDet;
    float w = 1 - u - v;


    // Initialize hit info
    TriangleHitInfo hitInfo;
    hitInfo.hit =  dst >= 0 && u >= 0 && v >= 0 && w >= 0;
    hitInfo.point = ray.origin + ray.dir * dst;
    hitInfo.normal = normalVec;
    hitInfo.dst = dst;

	return hitInfo;
}

vec3 getBackground(Ray ray)
{
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}


float rand() {
    float x = float(pixelCoords.x) / 1280.0f;
    float y = float(pixelCoords.y) / 720.0f;
    vec2 co = vec2(x, y);
    co.x *= seed + seed * randomSeed ;
    seed += 1;
    return 2 * (fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453) -0.5);
}
float rand_in_range(float min, float max)
{
	return min + (max - min) * rand();
}

vec3 random_vec3()
{
	return vec3(rand(), rand(), rand());
}

vec3 random_on_hemisphere(const vec3 normal)
{
    vec3 on_unit_sphere = random_vec3();
    if (dot(normal, on_unit_sphere) > 0.0)
	{
		return on_unit_sphere;
	}
	return -on_unit_sphere;
}

vec3 getEnviromentLight(Ray ray)
{
	vec3 skyColorHorizon = vec3(0.5, 0.7, 1.0);
    vec3 skyColorZenith = vec3(0.1, 0.1, 0.1);
    vec3 sunLightDir = (vec3(-0.5, -0.5, 0.5));
    float sunFocus = 0.1;
    float sunIntensity = 1.0;
    vec3 groundColor = vec3(0.3, 0.3, 0.3);

    float skyGradientT = pow(smoothstep(0.0, 0.4, ray.dir.y), 0.35);
    vec3 skyGradient = mix(skyColorZenith, skyColorHorizon, skyGradientT);
    float sun = pow(max(0, dot(ray.dir, -sunLightDir)), sunFocus) * sunIntensity;

    // Combine ground and sky
    float groundT = smoothstep(-0.01, 0, ray.dir.y);
    float sunMask = float(groundT >= 1);
    return mix(groundColor, skyGradient, groundT) + sunMask * sun;
}


HitInfo calculateRayCollision(Ray ray)
{
	HitInfo hitInfo;
	hitInfo.hit = false;
	hitInfo.dst = 9e9;


    // go over meshes
    for (int meshIndex = 0; meshIndex < meshes.length(); meshIndex++)
    {
        MeshInfo mesh = meshes[meshIndex];
        if (!hit_mesh(ray, mesh.boundsMin, mesh.boundsMax))
		{
			continue;
		}

        for (int i = 0; i < mesh.triangleCount; i++)
        {
            uint triIndex = mesh.triangleIndex + i;
            Triangle tri = triangles[triIndex];
            TriangleHitInfo hit = hit_triangle(ray, tri);

            if (hit.hit)
			{
                hitInfo.hit = true;
                hitInfo.dst = 0;
                hitInfo.material = Material(vec4(0,1,0,1), vec3(0), 0);
                hitInfo.point = hit.point;
                hitInfo.normal = hit.normal;
			}
        }



    }

    // Check spheres
	for (int i = 0; i < spheres.length(); i++)
	{
		Sphere sphere = spheres[i];
		HitInfo hit = hit_sphere(sphere.center, ray, sphere);
		if (hit.hit && hit.dst < hitInfo.dst)
		{
			hitInfo = hit;
		}
	}

	return hitInfo;
}

vec3 rayTrace(Ray ray)
{
    vec3 incomingLight = vec3(0);
    vec3 rayColor = vec3(1);

    // check triangles
    for (int i = 0; i < triangles.length()  +1; i++)
	{
		Triangle tri = triangles[i];
		TriangleHitInfo hit = hit_triangle(ray, tri);
		if (hit.hit)
        return RED;
    }

    // check meshes
    for (int meshIndex = 0; meshIndex < meshes.length(); meshIndex++)
    {
        MeshInfo mesh = meshes[meshIndex];
        if (!hit_mesh(ray, mesh.boundsMin, mesh.boundsMax))
		{
			continue;
		}
        return PURPLE;
}

    for (int i = 0; i < maxDepth; i++)
	{
		HitInfo hitInfo = calculateRayCollision(ray);

		if (hitInfo.hit)
		{
			ray.origin = hitInfo.point;
            ray.dir = normalize(hitInfo.normal + random_vec3());


            Material material = hitInfo.material;
            vec3 emittedLight = material.emissionStrength * material.emission;
            float lightStrength = dot(ray.dir, hitInfo.normal);
            incomingLight += emittedLight * rayColor.xyz * lightStrength * 2 * material.color.z;
            rayColor *= material.color.xyz ;


		}
		else
		{
            if (i == 0)
			{
				return getBackground(ray);
			}
			break;
		}
	}


    return incomingLight;
}


Ray createRay(int x, int y)
{
    vec3 pixel_center = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
    vec3 ray_dir = pixel_center - center;
    Ray r = Ray(center, ray_dir);
    return r;
}


vec3 blendLight(vec3 accumulatedLight, vec3 newLight, int frame) {
    if (frame == 1) {
        return newLight;
    } else {
        float factor = 1.0 / float(frame + 1);
        return mix(accumulatedLight, newLight, factor);
    }
}

void main()
{
    ivec2 imgSize = imageSize(imgOutput);

    Ray ray = createRay(pixelCoords.x, pixelCoords.y);
    vec3 totalIncomingLight = vec3(0);
    for (int i = 0; i < raysPerPixel; i++)
    {
        totalIncomingLight += rayTrace(ray);
    }
    totalIncomingLight /= float(raysPerPixel);

    vec3 oldLight = imageLoad(imgAccumulation, pixelCoords).xyz;
    totalIncomingLight = blendLight(oldLight, totalIncomingLight, frameNum);

    if (triangles[0].posB == vec3(2,-4,2))
    {
        totalIncomingLight = RED;
	
    }

    imageStore(imgAccumulation, pixelCoords, vec4(totalIncomingLight, 1.0));
    imageStore(imgOutput, pixelCoords, vec4(totalIncomingLight, 1.0));
}