#ifndef SHADERS_H_
#define SHADERS_H_

// even though gcc supports raw string literals it is still non standard
// so we can use this as a poor mans version of raw string literals
#define SHADER_CODE(...) "#version 460\n"  #__VA_ARGS__

// NOTE: this uses raw string literals which are a gcc extension
static char const *const vertex_shader_source = SHADER_CODE(
    out vec2 uv;

    void main()
    {
        uv = vec2[](vec2(-1, -1),
                    vec2(+1, -1),
                    vec2(-1, +1),
                    vec2(+1, +1))[gl_VertexID];

        gl_Position = vec4(uv, 0, 1);
    });

// based on https://github.com/RolandR/VolumeRayCasting/blob/master/webgl2/js/shaders/basic.frag
static char const *const fragment_shader_source = SHADER_CODE(
uniform int depth_steps;
uniform mat4 transform;
uniform sampler3D tex;

in vec2 uv;

out vec4 col;

vec3 aabb[2] = vec3[2](vec3(0.0, 0.0, 0.0),
                       vec3(1.0, 1.0, 1.0));

struct ray_t
{
    vec3 origin;
    vec3 direction;
    vec3 inv_direction;
    int sign[3];
};

ray_t make_ray(in vec3 origin, in vec3 direction)
{
    vec3 inv_direction = 1.0 / direction;
    return ray_t(origin,
                 direction,
                 inv_direction,
                 int[3](
                     ((inv_direction.x < 0.0) ? 1 : 0),
                     ((inv_direction.y < 0.0) ? 1 : 0),
                     ((inv_direction.z < 0.0) ? 1 : 0)));
}


void intersect(in ray_t ray, in vec3 aabb[2],
               out float tmin, out float tmax)
{
    float tymin, tymax, tzmin, tzmax;
    tmin = (aabb[ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
    tmax = (aabb[1-ray.sign[0]].x - ray.origin.x) * ray.inv_direction.x;
    tymin = (aabb[ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
    tymax = (aabb[1-ray.sign[1]].y - ray.origin.y) * ray.inv_direction.y;
    tzmin = (aabb[ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tzmax = (aabb[1-ray.sign[2]].z - ray.origin.z) * ray.inv_direction.z;
    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);
}

void main()
{
    vec4 origin = transform * vec4(0.0, 0.0, 2.0, 1.0) + 0.5;

    vec4 image = transform * vec4(uv, 4.0, 1.0) + 0.5;

    vec4 direction = normalize(origin - image);

    ray_t ray = make_ray(origin.xyz, direction.xyz);
    float tmin = 0.0;
    float tmax = 0.0;
    intersect(ray, aabb, tmin, tmax);

    if(tmin > tmax)
    {
        col = vec4(0);
        return;
    }

    vec3 start = origin.xyz + tmin*direction.xyz;
    vec3 end = origin.xyz + tmax*direction.xyz;

    float len = distance(end, start);
    int sample_count = int(float(depth_steps) * len);
    vec3 increment = (end - start) / float(sample_count);

    float inc_length = length(increment);
    increment = normalize(increment);

    vec3 pos = start;

    vec3 tex_coord = vec3(0.0, 0.0, 0.0);
    vec4 value = vec4(0.0, 0.0, 0.0, 0.0);

    for(int count = 0; count < sample_count; ++count)
    {
        tex_coord = mix(start, end, float(count) / float(sample_count));

        vec4 px = vec4(texture(tex, tex_coord).rrrr);
        value += px - px * px.a;

        if (value.a >= 0.99)
        {
            value.a = 1.0;
            break;
        }
    }

    col = value;
}
);

#endif //SHADERS_H_
