#include "scene.hpp"
#include "terrain.hpp"


using namespace cgp;

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
	image_structure image_skybox_template = image_load_file("assets/skybox_03.png");

	// Split the image into a grid of 4 x 3 sub-images
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);

	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[8], image_grid[6], image_grid[10], image_grid[4]);
	skybox.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, 3.14f / 2.0);

	// Create the shapes seen in the 3D scene
	// ********************************************** //

	float L = 7.0f;
	mesh terrain_mesh = mesh_primitive_grid({ -L,-L,0 }, { L,-L,0 }, { L,L,0 }, { -L,L,0 }, 100, 100);
	Terrain::deform_terrain(terrain_mesh);
	//mesh terrain_mesh = create_terrain_mesh(100,L);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg");

	float sea_w = 20.0;
	float sea_z = -5.0f;
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	//water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");
	water.material.color = { 0, 0, 1.0f };
	water.material.alpha = 0.2f;
	water.material.texture_settings.two_sided=true;
	water.shader.load(project::path + "shaders/mesh_deformation/mesh_deformation.vert.glsl", project::path + "shaders/mesh_deformation/mesh_deformation.frag.glsl");

	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	boat.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/boat/boat.obj"));
	//boat.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/fishing_boat/boat_diffuse.bmp", GL_REPEAT, GL_REPEAT);
	//boat.model.translation = vec3(2.0f,0,0);
	boat.model.scaling = 1.0f;
	boat.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, 3.14f / 2.0);

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

	grass_positions = Terrain::generate_positions_on_terrain(nb_grass, 3.0f*L, 0.0f);

	// mesh_load_file_obj: lit un fichier .obj et renvoie une structure mesh lui correspondant
	mesh tree_mesh = mesh_load_file_obj(project::path + "assets/mj/Plant.obj");

	// Initialisation classique de la structure mesh_drawable
	tree.initialize_data_on_gpu(tree_mesh);

	// Ajustement de la taille et position de la forme
	tree.model.scaling = 0.002f;
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, 3.14f / 2.0);
	tree_positions = Terrain::generate_positions_on_terrain(nb_tree, 3.0f*L, 0.0f);

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
	draw(boat,environment);

	for (int i = 0; i < nb_grass; i++) {
		grass.model.translation = grass_positions[i];
		draw(grass, environment);
	}

	for (int i = 0; i < nb_tree; i++) {
		tree.model.translation = tree_positions[i];
		draw(tree, environment);
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

