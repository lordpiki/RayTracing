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
    const float EPSILON = 1e-8;
    vec3 edge1 = tri.posB - tri.posA;
    vec3 edge2 = tri.posC - tri.posA;
    vec3 h = cross(ray.dir, edge2);
    float a = dot(edge1, h);

    TriangleHitInfo hitInfo;
    hitInfo.hit = false;

    if (abs(a) < EPSILON)
        return hitInfo; // Ray is parallel to the triangle

    float f = 1.0 / a;
    vec3 s = ray.origin - tri.posA;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return hitInfo;

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.dir, q);

    if (v < 0.0 || u + v > 1.0)
        return hitInfo;

    float t = f * dot(edge2, q);

    if (t > EPSILON)
    {
        hitInfo.hit = true;
        hitInfo.dst = t;
        hitInfo.point = ray.origin + ray.dir * t;
        hitInfo.normal = normalize(cross(edge1, edge2));

        if (dot(ray.dir, hitInfo.normal) > 0)
            hitInfo.normal = -hitInfo.normal;
    }

    return hitInfo;
}

vec3 getBackground(Ray ray)
{
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}


float rand() {
    float x = float(pixelCoords.x) / 1000.0f;
    float y = float(pixelCoords.y) / 1000.0f;
    vec2 co = vec2(x, y);
    co.x *= seed + randomSeed * seed;
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


HitInfo calculateRayCollision(Ray ray)
{
	HitInfo hitInfo;
	hitInfo.hit = false;
	hitInfo.dst = 9e9;

    // go over triangles
    for (int i = 0; i < triangles.length(); i++)
	{
		Triangle tri = triangles[i];
		TriangleHitInfo hit = hit_triangle(ray, tri);

		if (hit.hit && hit.dst < hitInfo.dst)
        {
            hitInfo.hit = true;
			hitInfo.dst = hit.dst;
			hitInfo.material = meshes[0].material;
			hitInfo.point = hit.point;
			hitInfo.normal = hit.normal;
		}
        }

        // go over meshes
        if (false)
        {
    for (int meshIndex = 0; meshIndex < meshes.length(); meshIndex++)
    {
        MeshInfo mesh = meshes[meshIndex];


        for (int i = 0; i < mesh.triangleCount; i++)
        {
            uint triIndex = mesh.triangleIndex + i;
            Triangle tri = triangles[triIndex];
            TriangleHitInfo hit = hit_triangle(ray, tri);
            if (!hit_mesh(ray, mesh.boundsMin, mesh.boundsMax))
		    {
			    continue;
		    }

            if (hit.hit && hit.dst < hitInfo.dst)
			{
                hitInfo.hit = true;
                hitInfo.dst = hit.dst;
                hitInfo.material = mesh.material;
                hitInfo.point = hit.point;
                hitInfo.normal = hit.normal;
                if (dot(ray.dir, hit.normal) > 0)
                {
                    hitInfo.normal = -hit.normal;
				}
			}
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

    for (int i = 0; i < maxDepth; i++)
	{
		HitInfo hitInfo = calculateRayCollision(ray);

		if (hitInfo.hit)
		{
			ray.origin = hitInfo.point;
            ray.dir = normalize(hitInfo.normal + random_vec3());

            Material material = hitInfo.material;
            vec3 emittedLight = material.emissionStrength * material.emission;
            incomingLight += emittedLight * rayColor.xyz;
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

    // randomize ray just a little bit
    ray_dir += random_vec3() * 0.002;


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

    imageStore(imgAccumulation, pixelCoords, vec4(totalIncomingLight, 1.0));
    imageStore(imgOutput, pixelCoords, vec4(totalIncomingLight, 1.0));
}