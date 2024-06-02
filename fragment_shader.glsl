#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform int width;
uniform int height;
uniform int numSpheres;

#define MAX_SPHERES 5
#define MAX_BOUNCE 6

struct Material
{
    vec3 color;
	float emission_strength;
};

struct HitInfo
{
    bool hit;
    float dst;
    vec3 point;
    vec3 normal;
    Material material;
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
	Material material;
};

layout(std140) uniform SphereBlock {
    Sphere spheres[MAX_SPHERES]; // Max num of spheres
};

float getRandomVal(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float getRandomValNormalDistribution(vec2 co)
{
	float theta = 2.0 * 3.1415926535897932384626433832795 * getRandomVal(co);
    float r = sqrt(-2.0 * log(getRandomVal(co)));
    return r * cos(theta);
}

vec3 getRandomVector(vec2 co)
{
	return normalize(vec3(getRandomValNormalDistribution(co), getRandomValNormalDistribution(co + vec2(1.0, 0.0)), getRandomValNormalDistribution(co + vec2(0.0, 1.0))));
}

vec3 randomHemisphereDir(vec3 normal, vec2 co)
{
    vec3 rand = getRandomVector(co);
	if (dot(rand, normal) < 0.0)
	{
		return -rand;
	}
	return rand;
}


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
    hitInfo.material = sphere.material;

    return hitInfo;
}

HitInfo calcRayCollision(Ray ray, Sphere[MAX_SPHERES] spheres)
{
    HitInfo closest;
    closest.hit = false;
    closest.dst = 1000000.0f;

    for (int i = 0; i < numSpheres; i++)
    {
        HitInfo hit = hit_sphere(spheres[i].center, ray, spheres[i]);
        if (hit.hit && hit.dst < closest.dst)
        {
            closest = hit;
        }
    }
    return closest;
}

vec3 trace(Ray ray, vec2 co)
{
    vec3 incomingLight = vec3(0, 0, 0);
    vec3 color  = vec3(1, 1, 1);

    for (int i = 0; i < MAX_BOUNCE; i++)
    {
        HitInfo hitInfo = calcRayCollision(ray, spheres);
        if (hitInfo.hit)
		{
			ray.origin = hitInfo.point;
			ray.dir = randomHemisphereDir(hitInfo.normal, co);;

            Material material = hitInfo.material;
            vec3 emission = material.emission_strength * material.color;
            incomingLight += color * emission;
            color *= material.color;
		}
		else
		{
            break;
		}
    }
    return incomingLight;
}

vec3 frag(Ray ray, Sphere spheres[MAX_SPHERES])
{
    return trace(ray, texCoord);

    
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

Ray ray_setup(int x, int y)
{
    float focal_length = 1.0;
    float viewport_height = 2.0;
    float viewport_width = viewport_height * (float(width) / height);
    vec3 camera_center = vec3(0, 0, 0);

    vec3 viewport_u = vec3(viewport_width, 0, 0);
    vec3 viewport_v = vec3(0, -viewport_height, 0);

    vec3 pixel_delta_u = viewport_u / float(width);
    vec3 pixel_delta_v = viewport_v / float(height);

    vec3 viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2.0f - viewport_v / 2.0f;
    vec3 pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

    vec3 pixel_center = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
    vec3 ray_direction = pixel_center - camera_center;

    Ray ray;
    ray.origin = camera_center;
    ray.dir = ray_direction;
    return ray;
}

void main()
{
    int x = int(texCoord.x * float(width));
    int y = int(texCoord.y * float(height));

    Ray ray = ray_setup(x, y);

    FragColor = vec4(frag(ray, spheres), 1.0);
}
