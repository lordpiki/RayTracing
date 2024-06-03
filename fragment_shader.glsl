#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform int width;
uniform int height;
uniform int numSpheres;

#define MAX_SPHERES 5
#define MAX_BOUNCE 2
#define RAYS_PER_PIXEL 2

struct Material
{
    vec3 color;
    float emission_strength;
    vec3 emmision_color;
    float reflection_strength;
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


HitInfo hit_sphere(vec3 center, Ray ray, Sphere sphere)
{
    HitInfo hitInfo;
    hitInfo.hit = false;
    hitInfo.dst = 1000000.0f;

    vec3 oc = ray.origin - center;
    float a = dot(ray.dir, ray.dir);
    float b = 2.0 * dot(oc, ray.dir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant > 0)
    {
        float temp = (-b - sqrt(discriminant)) / (2.0 * a);
        if (temp < hitInfo.dst && temp > 0.0001)
        {
            hitInfo.hit = true;
            hitInfo.dst = temp;
            hitInfo.point = ray.origin + ray.dir * temp;
            hitInfo.normal = (hitInfo.point - center) / sphere.radius;
            hitInfo.material = sphere.material;
        }
    }
    return hitInfo;
}

HitInfo calcRayCollision(Ray ray, Sphere spheres[MAX_SPHERES])
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

float getRandState(vec2 co)
{
    return (fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453));
}

float getRandomVal(int state)
{
    state = state * 747796405 + 2891336453;
    int res = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    res = (res >> 22) ^ res;
    return float(res) / float(0x7FFFFF3F);
}

float randomValNormalDist(int state)
{
    float theta = 2.0 * 3.141592653589793238 * getRandomVal(state);
    float rho = sqrt(-2 * log(getRandomVal(state)));
    return rho * cos(theta);
}


vec3 getRandomVector(int state)
{
    float x = randomValNormalDist(state);
    float y = randomValNormalDist(state + int(54532 * getRandomVal(state)));
    float z = randomValNormalDist(state + int(432425453 * getRandomVal(state)));
    return normalize(vec3(x, y, z));
}

vec3 randomHemisphereDir(vec3 normal, int state)
{
    
    vec3 dir = getRandomVector(state);
    return dir * sign(dot(dir, normal));
}


vec3 trace(Ray ray, int state)
{
    vec3 incomingLight = vec3(0, 0, 0);
    vec3 color  = vec3(1, 1, 1);
    

    for (int i = 0; i < MAX_BOUNCE; i++)
    {
        HitInfo hitInfo = calcRayCollision(ray, spheres);
        if (hitInfo.hit)
        {

            ray.origin = hitInfo.point;
            ray.dir = randomHemisphereDir(hitInfo.normal, state); // Random direction in hemisphere
            
            //return ray.dir;
            
            Material material = hitInfo.material;
            vec3 emission = material.emission_strength * material.emmision_color;
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
    vec3 totalLight = vec3(0, 0, 0);

    for (int i = 0; i < RAYS_PER_PIXEL; i++)
    {
        int state = int(getRandState(texCoord) * width * height + texCoord.xy * width * height);
        totalLight += trace(ray, abs(state));
    }

    return totalLight / RAYS_PER_PIXEL;
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
    vec3 ray_direction = normalize(pixel_center - camera_center);

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

    int numPixels = width * height;
    int pixelIndex = y * width + x;
    int state = pixelIndex;

    FragColor = vec4(frag(ray, spheres), 1.0);
}
