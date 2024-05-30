#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "terrain.hpp"
#include "key_positions_structure.hpp"

// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI (buttons, etc)
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;
};


// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	// A helper structure used to store and display the key positions/time
	keyframe_structure keyframe;

	// Timer used for the interpolation of the position
	cgp::timer_interval timer;

	cgp::skybox_drawable skybox;


	mesh_drawable terrain;
	mesh_drawable terrain2;
	mesh_drawable terrain3;
	mesh_drawable terrain4;
	mesh_drawable water;
	mesh_drawable grass;
	mesh_drawable boat;
	mesh_drawable tree;

	std::vector<cgp::vec3> grass_positions;
	std::vector<cgp::vec3> tree_positions;

	int nb_grass = 500;
	int nb_tree = 100;


	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop


	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

};





