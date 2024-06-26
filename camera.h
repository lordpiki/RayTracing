#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// using glm
using glm::vec3;
using glm::vec4;

struct ray
{
	vec3 center;
	vec3 dir;
};

struct Camera
{
public:
	double aspect_ratio = 16.0 / 9.0;
	int width;
	int height;
    int samples_per_pixel = 10;   // Count of random samples for each pixel
    int bounces = 10;   // Maximum number of ray bounces into scene

    double vfov = 90;  // Vertical view angle (field of view)
    vec3 lookfrom = vec3(0, 0, 0);   // Point camera is looking from
    vec3 lookat = vec3(-1, -1, -1);  // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);     // Camera-relative "up" direction
	
	// experimental
	vec3 dir = unit_vector(lookfrom - lookat);

    vec3 center = vec3(-1, 0, -1);
    vec3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;

	void init(int width_, int height_)
	{
		width = width_;
		height = height_;
        //pixel_samples_scale = 1.0 / samples_per_pixel;

        update_view();
	}

    void update_view()
    {
		//dir = unit_vector(lookfrom - lookat);
		// Determine viewport dimensions.
		auto focal_length = glm::length(dir);
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2 * h * focal_length;
		auto viewport_width = viewport_height * (double(width) / height);

		// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
		w = unit_vector(dir);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		vec3 viewport_u = float(viewport_width) * u;    // Vector across viewport horizontal edge
		vec3 viewport_v = float(viewport_height) * -v;  // Vector down viewport vertical edge

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / float(width);
		pixel_delta_v = viewport_v / float(height);

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left = center - (float(focal_length) * w) - viewport_u / 2.0f - viewport_v / 2.0f;
		pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);
    }

	static double degrees_to_radians(double degrees)
	{
		return degrees * 3.14159265358979323846 / 180.0;
	}

	static vec3 unit_vector(vec3 v)
	{
		return v / float(glm::length(v));
	}
	
private:
    vec3 u, v, w;              // Camera frame basis vectors


    

};