#version 330 core
out vec4 FragColor;
in vec2 texCoord;

// screen
uniform int width;
uniform int height;
uniform int frameCount;
uniform sampler2D accumTexture;


// camera 
uniform vec3 center;
uniform vec3 pixel00_loc;
uniform vec3 pixel_delta_u;
uniform vec3 pixel_delta_v;

// else
uniform int numSpheres;
uniform int maxDepth;
uniform int raysPerPixel;
#define MAX_SPHERES 5


int random_num = 0;


struct Material
{
	vec4 color;
    vec3 emission;
    float emissionStrength;
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

vec3 getBackground(Ray ray)
{
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

float rand(vec2 co)
{
    co.x += random_num + frameCount * random_num;
    random_num++;
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand_in_range(float min, float max)
{
    texCoord.x == texCoord.x ;
	return min + (max - min) * rand(texCoord);
}

vec3 random_vec3()
{
	return vec3(rand(texCoord), rand(texCoord), rand(texCoord));
}

vec3 random_in_unit_sphere()
{
    while (true) {
        vec3 p = random_vec3();
        float len = length(p);
        if (len * len < 1)
            return p;
    }
}

vec3 random_unit_vector()
{
    return normalize(random_in_unit_sphere());
}

vec3 random_on_hemisphere(const vec3 normal)
{
    vec3 on_unit_sphere = vec3(rand(texCoord), rand(texCoord), rand(texCoord));
    if (dot(on_unit_sphere, normal) > 0.0)
	{
		return on_unit_sphere;
	}
	return -on_unit_sphere;
}

HitInfo calculateRayCollision(Ray ray, Sphere spheres[MAX_SPHERES])
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

	return hitInfo;
}

vec3 rayTrace(Ray ray, Sphere spheres[MAX_SPHERES])
{
    vec3 incomingLight = vec3(0);
    vec3 rayColor = vec3(1);

    for (int i = 0; i < maxDepth; i++)
	{
		HitInfo hitInfo = calculateRayCollision(ray, spheres);

		if (hitInfo.hit)
		{
			ray.origin = hitInfo.point;
            ray.dir = random_on_hemisphere(hitInfo.normal);

            Material material = hitInfo.material;
            vec3 emittedLight = material.emissionStrength * material.emission;
            incomingLight += emittedLight * rayColor.xyz;
            rayColor *= material.color.xyz;
		}
		else
		{
            if (i == 0)
			{
				return getBackground(ray);
			}
            vec3 emittedLight = vec3(1, 1, 1);
            incomingLight += emittedLight * rayColor;
			break;
		}
	}

    if (incomingLight .x > 0 || incomingLight.y > 0 || incomingLight.z > 0)
		return incomingLight;

    return getBackground(ray);
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

    vec3 totalIncomingLight = vec3(0);
    for (int i = 0; i < raysPerPixel; i++)
    {
        totalIncomingLight += rayTrace(ray, spheres);
    }
    totalIncomingLight /= float(raysPerPixel);

    FragColor = vec4(totalIncomingLight, 1.0);  // Normal accumulated output


}