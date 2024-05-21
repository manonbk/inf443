#include "terrain.hpp"



using cgp::mesh;
//using cgp::mesh_drawable;
//using cgp::vec3;
//using cgp::numarray;
//using cgp::timer_basic;

float Terrain::evaluate_terrain_height(float x, float y) {

		//calcule la hauteur z de chaque point (x,y) du sol. Construit 4 �les selon des gaussiennes, modifi�es avec un bruit de perlin

		float cperl = 0.25;
		float cgauss = 2.5f;

		float d1 = x * x + y * y;
		float z1 = cgauss * exp(-d1 / 4) - 1;
		z1 = z1 + cperl * noise_perlin({ x,y }) + 0.3f;

		float d2 = (x - 4.8f) * (x - 4.8f) + (y - 4.8f) * (y - 4.8f);
		float z2 = cgauss * exp(-d2 / 6) - 1;
		z2 = z2 + cperl * noise_perlin({ x,y }) + 0.2f;

		float offset_3 = 5.0f;
		float d3 = (x - offset_3) * (x - offset_3) + (y + offset_3) * (y + offset_3);
		float z3 = cgauss * exp(-d3 / 3) - 1;
		z3 = z3 + cperl * noise_perlin({ x,y }) + 0.3f;

		float offset_4 = 3.0f;
		float d4 = (x + offset_4) * (x + offset_4) + (y + offset_4) * (y + offset_4);
		float z4 = cgauss * exp(-d4 / 3) - 1;
		z4 = z4 + cperl * noise_perlin({ x,y }) + 0.3f;

		return z1 + z2 + z3 + z4 + 0.8f;
	}

void Terrain::deform_terrain(mesh& m)
	{
		//applique la transformation calcul�e dans evaluate_terrain_height.

		for (int k = 0; k < m.position.size(); ++k)
		{
			vec3& p = m.position[k];
			float z = evaluate_terrain_height(p.x, p.y);

			p = { p.x, p.y, z };
		}

		m.normal_update();
	}

std::vector<cgp::vec3> Terrain::generate_positions_on_terrain(int N, float terrain_length, float height) {

		//cr�e un tableau de taille N de vecteurs 3d de positions
		std::vector<cgp::vec3> positions;
		float x;
		float y;
		float z;
		int count = 0;
		while (count < N) {
			x = float(std::rand()) / float(RAND_MAX) * terrain_length - terrain_length / 2;
			y = float(std::rand()) / float(RAND_MAX) * terrain_length - terrain_length / 2;
			z = evaluate_terrain_height(x, y);

			if (z > height) {
				positions.push_back({ x,y,z });
				count += 1;
			}
		}
		return positions;
	}


