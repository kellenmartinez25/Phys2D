// Phys2D.cpp : Defines the entry point for the application.
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

// engine object
Engine ENGINE;

int main()
{
	
	// Init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// create win
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Madman's 2D Physics", NULL, NULL);
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
	ResourceManager::loadShader("vertShader.vert", "fragShader.frag", nullptr, "shader");
	
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT), -1.0f, 1.0f);
	ResourceManager::getShader("shader").use().setMat4("projection", projection);
	Renderer* renderer = new Renderer(
		ResourceManager::getShader("shader")
	);
	// load test texture
	ResourceManager::loadTexture("resources/image.jpg", false, "image");
	
	DebugDraw::get().init();
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

	//ENGINE.addObject(
	//	new PhysObject(
	//		glm::dvec2(200.0, 200.0),
	//		glm::dvec2(100.0, 100.0),
	//		1.0f,
	//		30.0f,
	//		glm::dvec2(300.0, 100.0)
	//	)
	//);
	ENGINE.addObject(
		new PhysObject(
			glm::dvec2(400.0, 200.0),
			glm::dvec2(50.0, 100.0),
			1.0f,
			30.0f
		)
	);

	PhysObject borders[] = {
		PhysObject(
			glm::dvec2(0.0, 0.0),
			glm::dvec2(800.0, 100.0),
			0.0,
			true)
	};
	for (PhysObject& obj : borders) {
		ENGINE.addObject(&obj);
	}
	//for (int i = 0; i < nBoxes; i++) {
	//	// random x/y things
	//	float vx = rand(gen);
	//	float vy = rand(gen);
	//	// add new object
	//	ENGINE.addObject(new PhysObject(
	//		glm::dvec2(float(i) * d, 200.0f),	// evenly spaced
	//		glm::dvec2(100.0f + vx * 50.0f, 100.0f + vy * 50.0f),	// varying size
	//		((100.0f + vx * 50.0f) * (100.0f + vy * 50.0f)) / 10000.0f,
	//		glm::dvec2(vx*vScale, glm::abs(vy) * 10.0f)	// varying velocity
	//	));
	//}
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
		// debug draw
		DebugDraw::get().render();
		// swap buffers
		glfwSwapBuffers(window);
	}

	// run these on window close //

	// delete resources
	ResourceManager::clear();
	delete renderer;
	// terminate window
	glfwTerminate();

	return 0;
}

void error_callback(int err, const char* desc)
{
	fprintf(stderr, "Error %s\n", desc);
}
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}
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

