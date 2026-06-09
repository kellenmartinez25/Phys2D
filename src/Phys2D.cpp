// Phys2D.cpp : Main entry point for the simulator. Includes user input handling,
// setting up rendering, and contains the main engine/renderer update loop.
//

#include "Phys2D.h"
#include "PhysObject.h"
#include "DebugTools.h"

// default resolution
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// glfw callbacks
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void handleMouseCursor(GLFWwindow* win, double x, double y);
void handleMouseButton(GLFWwindow* win, int button, int action, int mods);
void handleScrollWheel(GLFWwindow* win, double dx, double dy);
void updateSelectedObject(GLFWwindow* win);


// declare scenarios, actual source is inside Scenarios.cpp
void scenario0(Engine* engine);
void scenario1(Engine* engine);
void scenario2(Engine* engine);
void scenario3(Engine* engine);

// the selected scenario
int scenarioSelect = 0;
// the number of scenarios we actually have
const int MAX_SCENARIOS = 3;

// replace all objects on the screen using the selected scenario
void placeObjects();

// engine object
Engine ENGINE;

// for placing object; this is rendered but not actually added to the engine
// its a preview of what you will place
PhysObject queuedObj;

int main()
{
	
	// Init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Phys2D", NULL, NULL);
	// check if it happened
	if (!window) {
		std::cerr << "Window creation FAILED" << std::endl;
		glfwTerminate();
		return -1;
	}
	// input callbacks
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, handleMouseCursor);
	glfwSetMouseButtonCallback(window, handleMouseButton);
	glfwSetScrollCallback(window, handleScrollWheel);

	// check if glad was initialized
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// print opengl versions to console
	GLint majVersion, minVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &majVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minVersion);
	std::cout << "OPENGL VERSION: " << majVersion << " : " << minVersion << std::endl;

	// OpenGL config
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// load shader
	ResourceManager::loadShader("vertShader.vert", "fragShader.frag", "shader");
	
	// init projection matrix
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT), -1.0f, 1.0f);
	// set the projection matrix
	ResourceManager::getShader("shader").use().setMat4("projection", projection);
	// create the renderer with our main shader
	Renderer* renderer = new Renderer(
		ResourceManager::getShader("shader")
	);
	// initialize the debug drawing
	DebugDraw::get().init();
	// set our projection matrix onto the debug tool draw
	ResourceManager::getShader("debugShader").use().setMat4("projection", projection);


	// set the engine's renderer and window to ours
	ENGINE.renderer = renderer;
	ENGINE.window = window;
	
	// place the objects for the first time
	placeObjects();

	// used for frame rate
	double dt;
	double lastFrame = glfwGetTime();
	double totFps = 0.0;
	double fpsHist[10]{ 0.0 };
	int f_i = 0;
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		double fps = 1.0 / dt;
		totFps -= fpsHist[f_i];
		fpsHist[f_i] = fps;
		totFps += fps;
		f_i = (f_i + 1) % 10;


		// poll inputs
		glfwPollEvents();
		// black bg
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// clear screen
		glClear(GL_COLOR_BUFFER_BIT);
		// update simulation with constant time step
		ENGINE.updateSim(0.005);

		// also put the queued/selected object to spawn in to the screen
		queuedObj.render(renderer);
		// debug draw
		DebugDraw::get().render();
		// refresh buffers to what we are currently drawing now
		glfwSwapBuffers(window);
		// cap to refresh rate
		glfwSwapInterval(1);
		if (f_i == 9) {
			std::cout << "\rObjects: " << ENGINE.objects.size() << "      FPS: " << totFps / 10.0 << "            " << std::flush;
		}
	}

	// run these on window close //

	// delete resources
	ResourceManager::clear();
	delete renderer;
	ENGINE.reset();
	// terminate window
	glfwTerminate();

	return 0;
}
// Place the objects depending on which scenario we have
void placeObjects() {
	// which scenario are we on? run the asssociated function
	switch (scenarioSelect) {
	default:
	case 0:
		scenario0(&ENGINE);
		break;
	case 1:
		scenario1(&ENGINE);
		break;
	case 2:
		scenario2(&ENGINE);
		break;
	case 3:
		scenario3(&ENGINE);
	}
}
// error
void error_callback(int err, const char* desc)
{
	fprintf(stderr, "Error %s\n", desc);
}
// handle key press
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	// close on pressing escape
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}
	// if we press a key down?
	if (action == GLFW_PRESS) {
		// R: reset
		if (key == GLFW_KEY_R) {
			ENGINE.reset();
			placeObjects();
		}
		// get current mouse pos
		double mouseX, mouseY;
		glfwGetCursorPos(win, &mouseX, &mouseY);
		// mouse Y transform to engine coords; 0 = bottom left
		mouseY = SCR_HEIGHT - mouseY;

		// +/- to scale
		if (key == GLFW_KEY_EQUAL) {
			queuedObj.size *= 1.1;
		}
		else if (key == GLFW_KEY_MINUS) {
			queuedObj.size *= 0.9;
		}
		// left/right to switch scenarios
		else if (key == GLFW_KEY_LEFT) {
			if (scenarioSelect > 0) {
				scenarioSelect--;
				ENGINE.reset();
				placeObjects();
			}
		}
		else if (key == GLFW_KEY_RIGHT) {
			if (scenarioSelect < MAX_SCENARIOS) {
				scenarioSelect++;
				ENGINE.reset();
				placeObjects();
			}
		}
		// G: show contact points. for debug purposes, defaults to off.
		else if (key == GLFW_KEY_G) {
			ENGINE.showContacts = !ENGINE.showContacts;
		}
	}
	// update the queued object with our new object values
	updateSelectedObject(win);
}

void handleMouseCursor(GLFWwindow* win, double x, double y) {
	// just refresh the thing
	updateSelectedObject(win);
}

void handleMouseButton(GLFWwindow* win, int button, int action, int mods) {
	// on press?
	if (action == GLFW_PRESS) {
		// left click: place it as a dynamic object
		if (button == GLFW_MOUSE_BUTTON_1) {
			// copy it to a new one
			updateSelectedObject(win);
			PhysObject* toAdd = new PhysObject();
			toAdd->pos = queuedObj.pos;
			toAdd->size = queuedObj.size;
			toAdd->angle = queuedObj.angle;
			toAdd->mass = queuedObj.mass;
			// also recalc inverses
			toAdd->calcInverses();
			ENGINE.addObject(toAdd);
		}
		// right click to add a static object
		if (button == GLFW_MOUSE_BUTTON_2) {
			// we clicked mouse2; add the object as a static
			PhysObject* toAdd = new PhysObject();
			toAdd->pos = queuedObj.pos;
			toAdd->size = queuedObj.size;
			toAdd->angle = queuedObj.angle;
			toAdd->isStatic = true; // its a right click
			toAdd->calcInverses();
			ENGINE.addObject(toAdd);
		}
	}
	
}

void handleScrollWheel(GLFWwindow* win, double dx, double dy) {
	// get the status of the ctrl/shift key
	bool shift = glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
	bool ctrl = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	// if we shifting or controlling, scale the object
	if (shift || ctrl) {
		// 2 statements to let you press both to scale in both directions
		if (shift) {
			queuedObj.size.x = std::max(queuedObj.size.x + dy * .1, .1);
		}
		if (ctrl) {
			queuedObj.size.y = std::max(queuedObj.size.y + dy * .1, .1);
		}
	}
	// advance the angle if not
	else {
		queuedObj.angle += dy * glm::radians(15.0);
	}
	// refresh preview
	updateSelectedObject(win);
}

// Update the previewed object to the mouse position
void updateSelectedObject(GLFWwindow* win) {
	double x, y;
	glfwGetCursorPos(win, &x, &y);
	// update the preview's screen position
	x /= 100.0;
	y = (SCR_HEIGHT - y) / 100.0;
	x -= queuedObj.size.x * 0.5;
	y -= queuedObj.size.y * 0.5;
	queuedObj.pos.x = (x);
	queuedObj.pos.y = y;
	// mass should be dependant on area
	queuedObj.mass = queuedObj.size.x * queuedObj.size.y;
	// also remember to update the inverse mass/inertia correctly
	queuedObj.calcInverses();
}

// handle window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	ENGINE.WIDTH = SCR_WIDTH;
	ENGINE.HEIGHT = SCR_HEIGHT;
	// also update the orthographic projection
	glm::mat4 newProj = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
	// put the new projection to our shader's uniforms
	ResourceManager::getShader("shader").use().setMat4("projection", newProj);
	ResourceManager::getShader("debugShader").use().setMat4("projection", newProj);
}

