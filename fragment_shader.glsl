#version 330 core
out vec4 FragColor;
in vec2 texCoord;
uniform int width;
uniform int height;

uniform vec3 sphereColor;
uniform vec3 sphereCenter;
uniform float sphereRadius;

struct HitInfo {
	bool hit;
    float dst;
    vec3 point;
    vec3 normal;
};

struct Ray {
	vec3 origin;
	vec3 dir;
};

struct Sphere
{
	vec3 color;
	vec3 center;
    float radius;
};


HitInfo hit_sphere(vec3 center, Ray ray, Sphere sphere)
{
	vec3 oc = center - ray.origin;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b*b - 4*a*c;

    HitInfo hitInfo;
    if (discriminant < 0)
	{
        
		hitInfo.hit = false;
        return hitInfo;
	}
    float dst = (-b - sqrt(discriminant)) / (2.0f * a);
	if (dst < 0.0f)
	{
		
		hitInfo.hit = false;
		return hitInfo;
	}

	hitInfo.hit = true;
	hitInfo.dst = dst;
	hitInfo.point = ray.origin + dst * ray.dir;
	hitInfo.normal = (hitInfo.point - center) / sphere.radius;
	return hitInfo;
}

vec3 rayTrace(Ray ray, Sphere sphere)
{   
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);

    HitInfo hit = hit_sphere(sphere.center, ray, sphere);
    if (hit.hit)
	{
		return hit.normal;
	}

	return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

Ray ray_setup(int x, int y)
{
    float focal_length = 1.0;
    float viewport_height = 2.0;
    float viewport_width = viewport_height * (float(width) / height);
    vec3 camera_center = vec3(0, 0, 0);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 viewport_u = vec3(viewport_width, 0, 0);
    vec3 viewport_v = vec3(0, -viewport_height, 0);


    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    vec3 pixel_delta_u = viewport_u / float(width);
    vec3 pixel_delta_v = viewport_v / float(height);
    
    // Calculate the location of the upper left pixel.
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

    Sphere sphere;
    sphere.color = sphereColor;
    sphere.center = sphereCenter;
    sphere.radius = sphereRadius;


    FragColor = vec4(rayTrace(ray, sphere), 1.0);
}
