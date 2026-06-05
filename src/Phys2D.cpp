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

// This is a type of function which accepts an Engine object and places objects inside them,
// for re-usable scenarios.
typedef void (*ScenarioFunc)(Engine* engine);

#define SCEN(n, g) scenario##n(g)
#define SCENSET(n) extern void scenario##n(Engine* engine)

// declare scenarios
extern void scenario0(Engine* engine);
extern void scenario1(Engine* engine);
extern void scenario2(Engine* engine);
extern void scenario3(Engine* engine);

int scenarioSelect = 0;
const int MAX_SCENARIOS = 3;

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
	// create win
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Phys2D", NULL, NULL);
	if (!window) {
		std::cerr << "Window creation FAILED" << std::endl;
		glfwTerminate();
		return -1;
	}
	//callbacks
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, handleMouseCursor);
	glfwSetMouseButtonCallback(window, handleMouseButton);
	glfwSetScrollCallback(window, handleScrollWheel);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	GLint majVersion, minVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &majVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minVersion);
	std::cout << "OPENGL VERSION: " << majVersion << " : " << minVersion;

	// OpenGL config
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//load shaders
	ResourceManager::loadShader("vertShader.vert", "fragShader.frag", "shader");
	
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT), -1.0f, 1.0f);
	ResourceManager::getShader("shader").use().setMat4("projection", projection);
	Renderer* renderer = new Renderer(
		ResourceManager::getShader("shader")
	);
	// initialize the debug drawing
	DebugDraw::get().init();
	// set our projection matrix onto the debug tool draw
	ResourceManager::getShader("debugShader").use().setMat4("projection", projection);

	//float dt = 0.0f;
	//float lastFrame = 0.0f;

	// set renderer and window things
	ENGINE.renderer = renderer;
	ENGINE.window = window;
	

	//random stuff
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> rand(-1.0f, 1.0f);
	// # of boxes to put
	int nBoxes = 5;
	// distance between boxes
	float d = static_cast<float>((ENGINE.WIDTH - 50.0f) / nBoxes);
	// what to scale our random value by for the velocity
	constexpr float vScale = 200.0f;

	placeObjects();
	// engine loop

	double dt;
	double lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// poll inputs
		glfwPollEvents();
		//black bg
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// clear screen
		glClear(GL_COLOR_BUFFER_BIT);
		// update simulation with constant time step
		ENGINE.updateSim(0.005);

		// also put the queued/selected object to spawn in to the screen
		queuedObj.render(renderer);
		// debug draw
		DebugDraw::get().render();
		// swap buffers
		glfwSwapBuffers(window);
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
void placeObjects() {
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
void error_callback(int err, const char* desc)
{
	fprintf(stderr, "Error %s\n", desc);
}
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_R) {
			ENGINE.reset();
			placeObjects();
		}
		double mouseX, mouseY;
		glfwGetCursorPos(win, &mouseX, &mouseY);
		mouseY = SCR_HEIGHT - mouseY;


		if (key == GLFW_KEY_EQUAL) {
			queuedObj.size *= 1.1;
		}
		if (key == GLFW_KEY_MINUS) {
			queuedObj.size *= 0.9;
		}
		if (key == GLFW_KEY_LEFT) {
			if (scenarioSelect > 0) {
				scenarioSelect--;
				ENGINE.reset();
				placeObjects();
			}
		}
		if (key == GLFW_KEY_RIGHT) {
			if (scenarioSelect < MAX_SCENARIOS) {
				scenarioSelect++;
				ENGINE.reset();
				placeObjects();
			}
		}
		if (key == GLFW_KEY_G) {
			ENGINE.showContacts = !ENGINE.showContacts;
		}
	}

	updateSelectedObject(win);
}

void handleMouseCursor(GLFWwindow* win, double x, double y) {
	updateSelectedObject(win);
	
}
void handleMouseButton(GLFWwindow* win, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (button == GLFW_MOUSE_BUTTON_1) {
			// we clicked mouse1; add the preview object to the engine
			// copy it to a new one
			updateSelectedObject(win);
			PhysObject* toAdd = new PhysObject();
			toAdd->pos = queuedObj.pos;
			toAdd->size = queuedObj.size;
			toAdd->angle = queuedObj.angle;
			toAdd->mass = queuedObj.mass;
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
			toAdd->isStatic = true;
			toAdd->calcInverses();
			ENGINE.addObject(toAdd);
		}
	}
	
}

void handleScrollWheel(GLFWwindow* win, double dx, double dy) {
	bool shift = glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
	bool ctrl = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	if (shift || ctrl) {
		// 2 statements to let you press both to scale in both directions
		if (shift) {
			queuedObj.size.x = std::max(queuedObj.size.x + dy * .1, .1);
		}
		if (ctrl) {
			queuedObj.size.y = std::max(queuedObj.size.y + dy * .1, .1);
		}
	}
	else {
		queuedObj.angle += dy * glm::radians(15.0);
	}
	updateSelectedObject(win);
}

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
	// mass should be dependant on surface area
	queuedObj.mass = queuedObj.size.x * queuedObj.size.y;
	// also remember to update the inverse mass/inertia correctly
	queuedObj.calcInverses();
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	ENGINE.WIDTH = SCR_WIDTH;
	ENGINE.HEIGHT = SCR_HEIGHT;
	// also update the orthographic projection
	glm::mat4 newProj = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
	ResourceManager::getShader("shader").use().setMat4("projection", newProj);
	ResourceManager::getShader("debugShader").use().setMat4("projection", newProj);
}

