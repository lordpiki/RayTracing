#version 330 core
out vec4 FragColor;
in vec2 texCoord;

// screen
uniform int width;
uniform int height;

// camera 
uniform vec3 center;
uniform vec3 pixel00_loc;
uniform vec3 pixel_delta_u;
uniform vec3 pixel_delta_v;

// else
uniform int numSpheres;
#define MAX_SPHERES 5

struct HitInfo
{
    bool hit;
    float dst;
    vec3 point;
    vec3 normal;
    vec3 color;
};

struct Ray
{
    vec3 origin;
    vec3 dir;
};

struct Sphere
{
    vec3 center;
    float radius;
    vec3 color;
    float pad; // padding to align with std140 layout
};

layout(std140) uniform SphereBlock {
    Sphere spheres[MAX_SPHERES]; // Max num of spheres
};

HitInfo hit_sphere(vec3 center, Ray ray, Sphere sphere)
{
    vec3 oc = center - ray.origin;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b*b - 4*a*c;

    HitInfo hitInfo;
    hitInfo.hit = false;

    if (discriminant < 0)
    {
        return hitInfo;
    }
    
    float dst = (-b - sqrt(discriminant)) / (2.0f * a);

    hitInfo.hit = true;
    hitInfo.dst = dst;
    hitInfo.point = ray.origin + dst * ray.dir;
    hitInfo.normal = (hitInfo.point - center) / sphere.radius;
    hitInfo.color = sphere.color;

    return hitInfo;
}

vec3 rayTrace(Ray ray, Sphere spheres[MAX_SPHERES])
{
	
    HitInfo hitInfo;
    hitInfo.hit = false;
    hitInfo.dst = 9e9;
    for (int i = 0; i < numSpheres; i++)
	{
		Sphere sphere = spheres[i];
		HitInfo hit = hit_sphere(sphere.center, ray, sphere);
		if (hit.hit && hit.dst < hitInfo.dst)
		{
			hitInfo = hit;
		}
	}
    if (hitInfo.hit)
	{
		return hitInfo.normal;
	}

    if (dot(ray.dir, vec3(0, 1, 0)) > 0.0f)
	{
		return vec3(0.5, 0.7, 1.0);
	}
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

Ray createRay(int x, int y)
{
    vec3 pixel_center = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
    vec3 ray_direction = pixel_center - center;
    Ray r = Ray(center, ray_direction);
    return r;
}


void main()
{
    int x = int(texCoord.x * float(width));
    int y = int(texCoord.y * float(height));

    Ray ray = createRay(x, y);

    FragColor = vec4(rayTrace(ray, spheres), 1.0);
}