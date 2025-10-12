vec3 sRGBToLinear(vec3 color)
{
	return mix(
		color * (1.0 / 12.92),
		pow(color * (1.0 / 1.055) + vec3(0.0521327), vec3(2.4)),
		step(0.04045, color)
	);
}