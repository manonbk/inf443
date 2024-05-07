#include "scene.hpp"


using namespace cgp;

float evaluate_terrain_height(float x, float y) {

	float cperl=0.25;
	float cgauss = 2.5f;

	float d1 = x * x + y * y;
	float z1 = cgauss*exp(-d1 / 4) - 1;
	z1 = z1 + cperl * noise_perlin({ x,y })+ 0.3f ;

	float d2 = (x - 4.8f) * (x - 4.8f) + (y - 4.8f) * (y - 4.8f);
	float z2 = cgauss*exp(-d2 / 6) - 1;
	z2 = z2 + cperl * noise_perlin({ x,y }) + 0.2f;

	float offset_3 = 5.0f;
	float d3 = (x - offset_3) * (x - offset_3) + (y + offset_3) * (y + offset_3);
	float z3 = cgauss*exp(-d3 / 3) - 1;
	z3 = z3 + cperl * noise_perlin({ x,y }) +0.3f ;

	float offset_4 = 3.0f;
	float d4 = (x + offset_4) * (x + offset_4) + (y + offset_4) * (y + offset_4);
	float z4 = cgauss*exp(-d4 / 3) - 1;
	z4 = z4 + cperl * noise_perlin({ x,y }) +0.3f;

	return z1+z2+z3+z4+0.8f;
}

void deform_terrain(mesh& m)
{
	// Set the terrain to have a gaussian shape
	for (int k = 0; k < m.position.size(); ++k)
	{
		vec3& p = m.position[k];
		float z = evaluate_terrain_height(p.x,p.y);

		p = { p.x, p.y, z };
	}

	m.normal_update();
}

std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_length) {
	//arbres
	std::vector<cgp::vec3> positions;
	float x;
	float y;
	float z;
	int count = 0;
	while (count < N) {
		x = float(std::rand()) / float(RAND_MAX) * terrain_length - terrain_length / 2;
		y = float(std::rand()) / float(RAND_MAX) * terrain_length - terrain_length / 2;
		z = evaluate_terrain_height(x, y);
		
		if (z > 0.0f) {
			positions.push_back({ x,y,z });
			count += 1;
		}
	}
	return positions;
}


// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	std::cout << "Start function scene_structure::initialize()" << std::endl;

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.look_at(
		{ 5.0f, -4.0f, 3.5f } /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);


	// Create the global (x,y,z) frame
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	//Skybox
	image_structure image_skybox_template = image_load_file("assets/skybox_02.jpg");

	// Split the image into a grid of 4 x 3 sub-images
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);

	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);
	
	// Create the shapes seen in the 3D scene
	// ********************************************** //

	float L = 7.0f;
	mesh terrain_mesh = mesh_primitive_grid({ -L,-L,0 }, { L,-L,0 }, { L,L,0 }, { -L,L,0 }, 100, 100);
	deform_terrain(terrain_mesh);
	//mesh terrain_mesh = create_terrain_mesh(100,L);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg");

	float sea_w = 8.0;
	float sea_z = -1.0f;
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");
	water.shader.load(project::path + "shaders/mesh_deformation/mesh_deformation.vert.glsl", project::path + "shaders/mesh_deformation/mesh_deformation.frag.glsl");

	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	// Create two quads to display the blades of grass as impostors
	mesh quad = mesh_primitive_quadrangle({ -0.5f,0.0f,0.0f }, { 0.5f,0.0f,0.0f }, { 0.5f,0.0f,1.0f }, { -0.5f,0.0f,1.0f });
	mesh quad2 = mesh_primitive_quadrangle({ 0.0f,-0.5f,0.0f }, { 0.0f,0.5f,0.0f }, { 0.0f,0.5f,1.0f }, { 0.0f,-0.5f,1.0f }); // second quad is orthogonal to the first one
	quad.push_back(quad2);
	grass.initialize_data_on_gpu(quad);
	grass.model.scaling = 0.1f;
	grass.material.phong = { 1,0,0,1 };
	grass.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/grass.png");

	// to use correctly the instancing, we will need a specific shader able to treat differently each instance of the shape
	grass.shader.load(project::path + "shaders/instancing/instancing.vert.glsl", project::path + "shaders/instancing/instancing.frag.glsl");

	grass_positions = generate_positions_on_terrain(100, L);

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	//  Must be called before drawing the other shapes and without writing in the Depth Buffer
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write

	// Update time
	timer.update();

	// Send current time as a uniform value to the shader
	environment.uniform_generic.uniform_float["time"] = timer.t;

	// conditional display of the global frame (set via the GUI)
	if (gui.display_frame)
		draw(global_frame, environment);
	

	// Draw all the shapes
	draw(terrain, environment);
	draw(terrain2,environment);
	draw(terrain3,environment);
	draw(terrain4, environment);
	draw(water, environment);
	draw(tree, environment);

	for (int i = 0; i < 100; i++) {
		grass.model.translation = grass_positions[i];
		draw(grass, environment);
	}


	// Animate the second cube in the water
	/*cube2.model.translation = {-1.0f, 6.0f + 0.1 * sin(0.5f * timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	draw(cube2, environment);*/

	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(terrain2, environment);
		draw_wireframe(terrain3, environment);
		draw_wireframe(water, environment);
		draw_wireframe(tree, environment);
		draw_wireframe(grass, environment);
	}
	







}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

