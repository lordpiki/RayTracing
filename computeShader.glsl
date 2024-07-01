#version 430 core
layout (local_size_x = 16, local_size_y = 16) in;
layout (rgba32f, binding = 0) uniform image2D imgOutput;
layout (rgba32f, binding = 1) uniform image2D imgAccumulation;

struct Material {
    vec4 color;
    vec3 emission;
    float emissionStrength;
};

struct Triangle{
    vec3 posA, posB, posC;
    vec3 normalA, normalB, normalC;
};

struct MeshInfo{
	uint triangleCount;
	uint materialIndex;
    vec3 boundsMin;
    vec3 boundsMax;
    Material material;
};


struct Sphere {
    vec3 center;
    float radius;
    Material material;
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

layout(std430, binding = 0) buffer SphereBuffer {
    Sphere spheres[];
};

layout(std430, binding = 1) buffer MeshBuffer {
	MeshInfo meshes[];
};

layout(std430, binding = 2) buffer TriangleBuffer {
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

    return tNear <= tFar && tFar > EPSILON;
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

    for (int i = 0; i < meshes.length(); i++)
    {
        MeshInfo mesh = meshes[i];
        if (hit_mesh(ray, mesh.boundsMin, mesh.boundsMax))
        {
            return mesh.material.color.xyz;
        }
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
            incomingLight += emittedLight * rayColor.xyz;
            rayColor *= material.color.xyz;

		}
		else
		{
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
    
    imageStore(imgAccumulation, pixelCoords, vec4(totalIncomingLight, 1.0));
    imageStore(imgOutput, pixelCoords, vec4(totalIncomingLight, 1.0));
}