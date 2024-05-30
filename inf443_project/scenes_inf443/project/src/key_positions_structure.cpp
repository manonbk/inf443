#include "key_positions_structure.hpp"

using namespace cgp;

void keyframe_structure::initialize(numarray<vec3> const& key_positions_arg, numarray<float> const& key_times_arg)
{
	key_positions = key_positions_arg;
	key_times = key_times_arg;

	// Initialise the mesh_drawable
	sphere_key_positions.initialize_data_on_gpu(mesh_primitive_sphere(0.05f));
	sphere_key_positions.material.color = { 1,1,1 };

	boat.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/Yatch_OBJ/Yatch.obj"));
	boat.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/Yatch_OBJ/Yatch_DIF.png", GL_REPEAT, GL_REPEAT);
	
	boat.model.scaling = 0.2f;
	boat.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, 3.14f / 2.0);
	boat.model.translation = vec3(2.0f,2.0f,0);
	
	

	polygon_key_positions.initialize_data_on_gpu(key_positions);
	polygon_key_positions.color = { 1,0,0 };
}

void keyframe_structure::display_key_positions(environment_structure const& environment)
{
	assert_cgp(key_times.size() == key_positions.size(), "key_time and key_positions should have the same size");

	// Display the key positions
	if (display_keyposition)
	{
		int N = key_positions.size();
		for (int k = 0; k < N; ++k) {
			if (picking.active && picking.index == k)
				sphere_key_positions.material.color = { 1,0,0 };
			else
				sphere_key_positions.material.color = { 1,1,1 };
			sphere_key_positions.model.translation = key_positions[k];
			draw(sphere_key_positions, environment);
		}
	}
		

	// Display the polygon linking the key positions
	if (picking.active)
		polygon_key_positions.vbo_position.update(key_positions); // update the polygon if needed
	if (display_polygon)
		draw(polygon_key_positions, environment);

}

void keyframe_structure::display_current_position(vec3 const& p,float t, environment_structure& environment)
{
	// Display the interpolated position
	boat.model.translation = p;
	//vec2 p0 = {2.0f,2.0f};
	float d = sqrt(p.x*p.x+p.y*p.y);
	float omega = 20.0*d - 3.0*t;
	vec3 p2 = vec3(p.x, p.y, 0.05*cos(omega));
	boat.model.translation = p2;
	draw(boat, environment);

	// Display the trajectory
	trajectory.visual.color = { 0,0,1 };
	trajectory.add(p);
	if (display_trajectory)
		draw(trajectory, environment);
}

void keyframe_structure::update_picking(input_devices const& inputs, camera_generic_base const& camera, camera_projection_perspective const& projection)
{
	// Current position of the mouse
	vec2 const& p = inputs.mouse.position.current;

	// The picking and deformation is only applied when pressing the shift key
	if (inputs.keyboard.shift)
	{
		// If the mouse is not clicked, compute a picking on the vertices of the grid
		if (!inputs.mouse.click.left)
			picking = picking_spheres(p, key_positions, 0.05f, camera, projection);

		// Key position translation
		if (inputs.mouse.click.left && picking.active)
		{
			vec3 new_position = picking_plane_orthogonal_to_camera(p, picking.position, camera, projection).position;
			key_positions[picking.index] = new_position;
		}
	}
	else
		picking.active = false;
}

void keyframe_structure::display_gui()
{
	ImGui::Checkbox("Display key positions", &display_keyposition);
	ImGui::Checkbox("Display polygon", &display_polygon);
	ImGui::Checkbox("Display trajectory", &display_trajectory);
	bool new_size = ImGui::SliderInt("Trajectory size", &trajectory_storage, 2, 500);

	if (new_size) {
		trajectory.clear();
		trajectory = trajectory_drawable(trajectory_storage);
	}
}