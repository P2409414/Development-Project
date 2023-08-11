#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <imgui_internal.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"

#include <Model.h>

#include<string>
#include <iostream>
#include <numeric>

struct Animator
{
	float alpha;
	bool tick;
	float speed;
};

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
static int index = 0;

char buf[50];
char buf1[50];
string path = "";
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);
//unsigned int loadTexture2(char const * path);
void setVAO(vector <float> vertices);
void initialiseMenu(GLFWwindow* window);
void animateContent(Animator& animator);
void centerButtons(std::vector<std::string> names, std::vector<int> indexs, int& selected_index);
void centerText(const char* format, const float y_padding, ImColor colour);
void customCheckbox(const char* format, bool* value);
void renderMenu();
void renderControls(GLFWwindow* window);
ImFont* rubikTitle;
ImFont* rubikButton;
bool information = true;
bool modelSelect = false;
bool check = false;
bool check2 = false;
bool collapse = false;
bool CKey = false;
bool menu = true;
bool load = false;
unsigned int quadVAO, VBO, VAO;
unsigned int ppFBO, blurFBO, blurAttachment, colourAttachment[2], depthAttachment;

glm::vec3 m_lightDir = glm::vec3(0, -1, 0);
glm::vec3 m_lightColour = glm::vec3(1.0, 1.0, 1.0);

// camera
Camera camera(glm::vec3(0, 0, 9));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void createQuad() {
	float quadVertices[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void drawQuad(Shader shader, unsigned int& textureObj) {
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureObj);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void setFBO() {
	glGenFramebuffers(1, &ppFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ppFBO);
	glGenTextures(2, colourAttachment);
	for (int i = 0; i < 3; i++) {
		glBindTexture(GL_TEXTURE_2D, colourAttachment[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colourAttachment[i], 0);
	}

	glGenTextures(1, &depthAttachment);
	glBindTexture(GL_TEXTURE_2D, depthAttachment);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurFBO() {
	glGenFramebuffers(1, &blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glGenTextures(1, &blurAttachment);
	glBindTexture(GL_TEXTURE_2D, blurAttachment);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurAttachment, 0);

	glGenTextures(1, &depthAttachment);
	glBindTexture(GL_TEXTURE_2D, depthAttachment);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// simple vertex and fragment shader 
	//Shader shader("..\\shaders\\basicVert.vs", "..\\shaders\\basicFrag.fs", "..\\shaders\\basicGeo.gs");
	Shader shader("..\\shaders\\modelVert.vs", "..\\shaders\\modelFrag.fs");
	Shader normShader("..\\shaders\\modelVert.vs", "..\\shaders\\normFrag.fs", "..\\shaders\\normGeo.gs");
	Shader postProccessing("..\\shaders\\postVert.vs", "..\\shaders\\postFrag.fs");
	Shader blurShader("..\\shaders\\postVert.vs", "..\\shaders\\blurFrag.fs");
	Model nano("..\\resources\\nano\\nanosuit.obj");
	createQuad();
	setFBO();
	BlurFBO();

	while (!glfwWindowShouldClose(window))
	{
		if (check2 == false)
		{
			initialiseMenu(window);
			check2 = true;
		}
		if (check == false)
		{
			if (path != "")
			{
				Model nano(path);
				check = true;
			}
		}
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		blurShader.use();
		blurShader.setMat4("projection", projection);
		blurShader.setMat4("view", view);
		blurShader.setMat4("model", model);
		shader.use();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (menu == true)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			renderMenu();
		}
		else
		{
			//ImGui::Render();
			//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		if (load == true)
		{
			processInput(window);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glBindFramebuffer(GL_FRAMEBUFFER, ppFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			shader.setVec3("lightCol", m_lightColour);
			shader.setVec3("lightDir", m_lightDir);

			//Point Light

			shader.setVec3("pLight[0].position", glm::vec3(2.0, 3.0, 4.0));
			shader.setVec3("pLight[0].colour", m_lightColour);
			shader.setFloat("pLight[0].Kc", 1.0f);
			shader.setFloat("pLight[0].Kl", 0.22f);
			shader.setFloat("pLight[0].Ke", 0.2f);

			shader.setVec3("pLight[1].position", glm::vec3(-2.0, -3.0, -4.0));
			shader.setVec3("pLight[1].colour", m_lightColour);
			shader.setFloat("pLight[1].Kc", 1.0f);
			shader.setFloat("pLight[1].Kl", 0.22f);
			shader.setFloat("pLight[1].Ke", 0.2f);

			//Spot Light
			shader.setVec3("sLight.position", camera.Position);
			shader.setVec3("sLight.direction", (camera.Front));
			shader.setVec3("sLight.colour", glm::vec3(1.0, 1.0, 1.0));
			shader.setFloat("sLight.Kc", 1.0f);
			shader.setFloat("sLight.Kl", 0.0002f);
			shader.setFloat("sLight.Ke", 0.0002f);
			shader.setFloat("sLight.innerRad", glm::cos(glm::radians(15.5f)));
			shader.setFloat("sLight.outerRad", glm::cos(glm::radians(17.5f)));

			nano.Draw(shader);
			glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
			glDisable(GL_DEPTH_TEST);
			//Blur shader
			blurShader.use();
			//nano.Draw(normShader);
			blurShader.setInt("horz", 1);
			drawQuad(blurShader, colourAttachment[0]);
			blurShader.setInt("horz", 0);
			drawQuad(blurShader, blurAttachment);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			drawQuad(postProccessing, colourAttachment[0]);
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				drawQuad(postProccessing, blurAttachment);
			}
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			renderControls(window);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		information = true;
		modelSelect = false;
		load = false;
		menu = true;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		m_lightColour = glm::vec3(1.0, 1.0, 1.0);
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		m_lightColour = glm::vec3(1.0, 0.0, 0.0);
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		m_lightColour = glm::vec3(0.0, 1.0, 0.0);
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		m_lightColour = glm::vec3(0.0, 0.0, 1.0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void initialiseMenu(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGuiStyle& style = ImGui::GetStyle();
	auto& colours = style.Colors;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	rubikTitle = io.Fonts->AddFontFromFileTTF("..\\resources\\fonts\\Rubik-Bold.ttf", 40.0f);
	rubikButton = io.Fonts->AddFontFromFileTTF("..\\resources\\fonts\\Rubik-Regular.ttf", 25.0f);

	style.ScrollbarRounding = 0;
	style.WindowRounding = 4.0f;
	colours[ImGuiCol_ResizeGrip] = ImColor(0, 0, 0, 0);
	colours[ImGuiCol_ResizeGripActive] = ImColor(0, 0, 0, 0);
	colours[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0);

	colours[ImGuiCol_Button] = ImColor(18, 18, 18, 100);
	colours[ImGuiCol_ButtonActive] = ImColor(21, 21, 21, 100);
	colours[ImGuiCol_ButtonHovered] = ImColor(21, 21, 21, 100);

	colours[ImGuiCol_CheckMark] = ImColor(0, 189, 0, 255);

	colours[ImGuiCol_FrameBg] = ImColor(24, 24, 24);
	colours[ImGuiCol_FrameBgActive] = ImColor(26, 26, 26);
	colours[ImGuiCol_FrameBgHovered] = ImColor(26, 26, 26);
}

void animateContent(Animator& animator)
{
	const int limit = 255;
	if (animator.tick || animator.alpha == limit)
	{
		animator.tick = true;
		if (!(animator.alpha <= 0))
			animator.alpha -= animator.speed;
		else if (animator.alpha <= 0)
			animator.tick ^= 1;
	}
	else if (!animator.tick || animator.alpha != limit)
	{
		animator.tick = false;
		if (!(animator.alpha >= limit))
			animator.alpha += animator.speed;
		else if (animator.alpha >= limit)
			animator.tick ^= 1;
	}
}

void centerButtons(std::vector<std::string> names, std::vector<int> indexs, int& selected_index)
{
	std::vector<ImVec2> sizes = {};
	float total_area = 0.0f;

	const auto& style = ImGui::GetStyle();

	for (std::string& name : names) {
		const ImVec2 label_size = ImGui::CalcTextSize(name.c_str(), 0, true);
		ImVec2 size = ImGui::CalcItemSize(ImVec2(), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

		size.x += 45.5f;
		size.y += 15.f;

		sizes.push_back(size);
		total_area += size.x;
	}

	ImGui::SameLine(15.f + ((ImGui::GetContentRegionAvail().x / 2) - (total_area / 2)));
	for (uint32_t i = 0; i < names.size(); i++) {
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

		if (selected_index == indexs[i]) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
			if (ImGui::Button(names[i].c_str(), sizes[i]))
				selected_index = indexs[i];
			ImGui::PopStyleColor(3);
		}
		else {
			if (ImGui::Button(names[i].c_str(), sizes[i]))
				selected_index = indexs[i];
		}

		ImGui::PopStyleVar();
		if (i != (names.size() - 1))
			ImGui::SameLine();
	}
}

void centerText(const char* format, const float y_padding = 0.0f, ImColor colour = ImColor(255, 255, 255)) 
{
	const ImVec2 text_size = ImGui::CalcTextSize(format);

	ImGui::SameLine(15.f + ((ImGui::GetContentRegionAvail().x / 2) - (text_size.x / 2)));
	if (y_padding > 0.0f)
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_padding);
	ImGui::TextColored(colour, format);
}

void customCheckbox(const char* format, bool* value)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.5f, 1.5f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImColor(41, 74, 122, 255).Value);
	ImGui::Checkbox(format, value);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::Dummy(ImVec2());
}

void renderMenu()
{

	static Animator animator{ 255, false, 0.1f };
	animateContent(animator);

	ImGui::SetNextWindowSize({ 600, 400 });
	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImVec2 window_pos{ ImGui::GetWindowPos() };
	ImVec2 window_size{ ImGui::GetWindowSize() };
	ImVec2 cursor_pos{ ImGui::GetCursorPos() };
	ImGui::PushFont(rubikTitle);
	centerText("MENU");
	ImGui::PopFont();
	ImGui::PushFont(rubikButton);
	centerButtons({ "English", "Francais"}, {0, 1}, index);

	ImGui::PushStyleColor(ImGuiCol_Separator, ImColor(41, 74, 122).Value);
	ImGui::Separator();
	ImGui::PopStyleColor();

	ImGui::BeginChild("##left_side", ImVec2(ImGui::GetContentRegionAvail().x / 2.0f, ImGui::GetContentRegionAvail().y));
	{
		if (index == 0)
		{
			ImVec2 size = ImGui::CalcItemSize(ImVec2(), ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 15.0f);
			size.y += 15.f;
			if (information == true) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Information", ImVec2(size.x, size.y)))
				{
					information = true;
					modelSelect = false;
				}
				ImGui::PopStyleColor(3);
				if (ImGui::Button("Model Select", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = true;
				}
				ImGui::Dummy(ImVec2(0, 160));
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				if (ImGui::Button("Start", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = false;
					load = true;
					menu = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
			else if (modelSelect == true)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Information", ImVec2(size.x, size.y)))
				{
					information = true;
					modelSelect = false;
				}
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				if (ImGui::Button("Model Select", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = true;
				}
				ImGui::Dummy(ImVec2(0, 160));
				if (ImGui::Button("Start", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = false;
					load = true;
					menu = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
		}
		else if (index == 1)
		{
			ImVec2 size = ImGui::CalcItemSize(ImVec2(), ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 15.0f);
			size.y += 15.f;
			if (information == true) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 1220, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Information", ImVec2(size.x, size.y)))
				{
					information = true;
					modelSelect = false;
				}
				ImGui::PopStyleColor(3);
				if (ImGui::Button("Selection du modele", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = true;
				}
				ImGui::Dummy(ImVec2(0, 160));
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				if (ImGui::Button("Commencer", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = false;
					load = true;
					menu = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
			else if (modelSelect == true)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Information", ImVec2(size.x, size.y)))
				{
					information = true;
					modelSelect = false;
				}
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				if (ImGui::Button("Selection du modele", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = true;
				}
				ImGui::Dummy(ImVec2(0, 160));
				if (ImGui::Button("Commencer", ImVec2(size.x, size.y)))
				{
					information = false;
					modelSelect = false;
					load = true;
					menu = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Separator, ImColor(41, 74, 122, 255).Value);
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::BeginChild("##right_side", ImVec2(ImGui::GetContentRegionAvail().x / 1.0f, ImGui::GetContentRegionAvail().y));
	{
		if (index == 0)
		{
			if (information == true)
			{
				ImGui::TextWrapped("Welcome to the model viewer, this program has been made so you can load your models and see the effects of different lighting and shading types on them.");
				ImGui::Dummy(ImVec2());
				ImGui::TextWrapped("In order to change the model viewed switch to the model select tab.");
				ImGui::Dummy(ImVec2());
				ImGui::TextWrapped("Set your custom models up in the 'Resources' folder like 'nano'.");
			}
			else if (modelSelect == true)
			{
				ImGui::TextWrapped("Please enter the folder name of your model.");
				ImGui::InputText("##text1", buf, 100);
				ImGui::TextWrapped("Please enter the obj file name of your model with      '.obj'");
				ImGui::InputText("##text2", buf1, 100);
				ImVec2 size = ImGui::CalcItemSize(ImVec2(), ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 5.0f);
				size.y += 15.f;
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Accept", ImVec2(size.x, size.y)))
				{
					string pt = buf;
					string pt1 = buf1;
					path = "..\\resources\\" + pt + "\\" + pt1;
					std::cout << path << std::endl;
					check = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
		}
		else if (index == 1)
		{
			if (information == true)
			{
				ImGui::TextWrapped("Bienvenue dans la visionneuse de modeles, ce programme a ete concu pour que vous puissiez charger vos modeles et voir les effets de differents types d'eclairage et d'ombrage sur eux.");
				ImGui::Dummy(ImVec2());
				ImGui::TextWrapped("Pour changer le modèle affiche, passez a l'onglet de selection du modele.");
				ImGui::Dummy(ImVec2());
				ImGui::TextWrapped("Configurez vos modeles personnalises dans le dossier 'Ressources' comme 'nano'.");
			}
			else if (modelSelect == true)
			{
				ImGui::TextWrapped("Veuillez saisir le nom du dossier de votre modele.");
				ImGui::InputText("##text1", buf, 100);
				ImGui::TextWrapped("Veuillez entrer le nom du fichier obj de votre modele avec '.obj'");
				ImGui::InputText("##text2", buf1, 100);
				ImVec2 size = ImGui::CalcItemSize(ImVec2(), ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 5.0f);
				size.y += 15.f;
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(41, 74, 122, 255).Value);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				if (ImGui::Button("Accepter", ImVec2(size.x, size.y)))
				{
					string pt = buf;
					string pt1 = buf1;
					path = "..\\resources\\"+pt+"\\"+pt1;
					std::cout << path << std::endl;
					check = false;
				}
				ImGui::PopStyleColor(3);
				ImGui::PopStyleVar();
			}
		}
	}
	ImGui::EndChild();

	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddRect({ window_pos.x - 1, window_pos.y - 1 }, { window_pos.x + window_size.x + 1, window_pos.y + window_size.y + 1 }, ImColor(41, 74, 122, (int)animator.alpha), 4.0f, 0, 2.0f);

	ImGui::PopFont();
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderControls(GLFWwindow* window)
{

	static Animator animator{ 255, false, 0.1f };
	animateContent(animator);
	ImGui::PushFont(rubikButton);
	ImGui::SetNextWindowSize({ 400, 400 });
	if (index == 0)
	{
		ImGui::Begin("Controls press C", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	}
	else if (index == 1)
	{
		ImGui::Begin("Controles appuyez sur C", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	}
	ImVec2 window_pos{ ImGui::GetWindowPos() };
	ImVec2 window_size{ ImGui::GetWindowSize() };
	ImVec2 cursor_pos{ ImGui::GetCursorPos() };
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		if (CKey == false)
		{
			CKey = true;
			if (collapse == false)
			{
				ImGui::SetWindowCollapsed(true);
				collapse = true;
			}
			else
			{
				ImGui::SetWindowCollapsed(false);
				collapse = false;
			}
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
		if (CKey == true)
		{
			CKey = false;
		}
	}
	ImGui::BeginChild("##left_side", ImVec2(ImGui::GetContentRegionAvail().x / 4.0f, ImGui::GetContentRegionAvail().y));
	ImGui::Text("ESC ");
	ImGui::Text("M");
	ImGui::Text("WASD");
	if (index == 0)
	{
		ImGui::Text("Mouse");
	}
	else if (index == 1)
	{
		ImGui::Text("La souris");
	}
	ImGui::Text("Q");
	ImGui::Text("G");
	ImGui::Text("H");
	ImGui::Text("J");
	ImGui::Text("K");
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Separator, ImColor(41, 74, 122, 255).Value);
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::BeginChild("##right_side", ImVec2(ImGui::GetContentRegionAvail().x / 1.0f, ImGui::GetContentRegionAvail().y));
	if (index == 0)
	{
		ImGui::Text("Exit program");
		ImGui::Text("Enter menu");
		ImGui::Text("Camera movement");
		ImGui::Text("Camera movement");
		ImGui::Text("Blur");
		ImGui::Text("White light");
		ImGui::Text("Red light");
		ImGui::Text("Green light");
		ImGui::Text("Blue light");
	}
	else if (index == 1)
	{
		ImGui::Text("Quitter le programme");
		ImGui::Text("Entrer dans le menu");
		ImGui::Text("Mouvement de camera");
		ImGui::Text("Mouvement de camera");
		ImGui::Text("Se brouiller");
		ImGui::Text("Lumiere blanche");
		ImGui::Text("Lumiere rouge");
		ImGui::Text("Lumiere verte");
		ImGui::Text("Lumiere bleue");
	}
	ImGui::EndChild();
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	draw_list->AddRect({ window_pos.x - 1, window_pos.y - 1 }, { window_pos.x + window_size.x + 1, window_pos.y + window_size.y + 1 }, ImColor(41, 74, 122, (int)animator.alpha), 4.0f, 0, 2.0f);
	ImGui::PopFont();
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}