#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform int width;
uniform int height;
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

    if (closest.hit)
    {
        return closest.normal;
    }

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

    FragColor = vec4(rayTrace(ray, spheres), 1.0);
}
