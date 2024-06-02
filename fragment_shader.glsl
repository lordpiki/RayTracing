#version 330 core
out vec4 FragColor;
in vec2 texCoord;
uniform int width;
uniform int height;
in vec3 pixel00_loc;

struct Ray {
	vec3 origin;
	vec3 dir;
};

vec3 rayTrace(Ray ray)
{   
    vec3 unit_dir = normalize(ray.dir);
    float t = 0.5f * (unit_dir.y + 1.0f);

	return (1.0 - t) * vec3(1, 1, 1) + t * vec3(0.5, 0.7, 1.0);
}

void main() 
{
    int x = int(texCoord.x * float(width));
    int y = int(texCoord.y * float(height));

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
    FragColor = vec4(rayTrace(ray), 1.0);
}
