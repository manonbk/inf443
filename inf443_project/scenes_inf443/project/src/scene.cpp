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

	return z1+z2+z3+z4;
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

	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	cube1.initialize_data_on_gpu(mesh_primitive_cube({ 0,0,0 }, 0.5f));
	cube1.model.rotation = rotation_transform::from_axis_angle({ -1,1,0 }, Pi / 7.0f);
	cube1.model.translation = { 1.0f,1.0f,-0.1f };
	cube1.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/wood.jpg");

	cube2 = cube1;

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	// Update time
	timer.update();

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
	draw(cube1, environment);

	// Animate the second cube in the water
	cube2.model.translation = { -1.0f, 6.0f+0.1*sin(0.5f*timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	draw(cube2, environment);

	if (gui.display_wireframe) {
		draw_wireframe(terrain, environment);
		draw_wireframe(terrain2, environment);
		draw_wireframe(terrain3, environment);
		draw_wireframe(water, environment);
		draw_wireframe(tree, environment);
		draw_wireframe(cube1, environment);
		draw_wireframe(cube2, environment);
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

