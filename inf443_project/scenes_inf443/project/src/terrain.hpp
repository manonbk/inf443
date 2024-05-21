#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"


class Terrain {

public:
	static float evaluate_terrain_height(float x, float y);

	static void deform_terrain(cgp::mesh& m);

	static std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_length, float height);

};