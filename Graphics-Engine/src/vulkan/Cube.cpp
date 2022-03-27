#include <array>
#include "Cube.hpp"
#include "Pipeline.hpp"
#include "../utils/Logger.hpp"
#include <glm/gtc/constants.hpp>

namespace Vk
{
	Cube::Cube(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, 
		const VkCommandPool commandPool)
		:Renderable(device, vertices, indices, commandPool)
	{
	}

	Cube::~Cube() noexcept
	{

	}

	void Cube::draw(VkCommandBuffer commandBuffer, const VkPipelineLayout pipelineLayout) const 
	{
		VkBuffer rawVertexBuffer = vertexBuffer->getBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &rawVertexBuffer, &offset);

		//vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		PushConstant push{};
		push.view = glm::mat4{ 1.0f };
		push.model = transform.getModel();
		//static unsigned long long ctr = 0;
		//push.model = glm::rotate(push.model, glm::radians(.05f * ctr++), glm::vec3{ 0, 1, 0 });
		//push.model = glm::rotate(push.model, glm::radians(.01f * ctr++), glm::vec3{ 1, 0, 0 });
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);

        //vkCmdDrawIndexed(commandBuffer, indexBuffer->getVertexCount(), 1, 0, 0, 0);
		vkCmdDraw(commandBuffer, vertexBuffer->getVertexCount(), 1, 0, 0);
		//transform.rotation.x += 0.0005f;
		transform.rotation.y = glm::mod(transform.rotation.y + 0.001f, glm::two_pi<float>());
		transform.rotation.z = glm::mod(transform.rotation.z + 0.0005f, glm::two_pi<float>());
	}

	Cube Cube::createCube(const Device& device, const glm::vec3& dimensions, const glm::vec3& position, const glm::vec3& color, const VkCommandPool commandPool)
	{
		std::vector<Vertex> vertices(8);
		std::vector<uint32_t> indices(36);

		std::array gradientColors 
		{
			glm::vec3{ 1.0f, 0.0f, 0.0f }, 
			glm::vec3{ 0.0f, 1.0f, 0.0f }, 
			glm::vec3{ 0.0f, 0.0f, 1.0f }, 
			glm::vec3{ 1.0f, 1.0f, 1.0f }
		};
		float x= -(dimensions.x / 2.0f), y= -(dimensions.y / 2.0f), z= dimensions.z / 2.0f;
		size_t cIdx = 0;
		for (size_t i = 0; i < vertices.size(); ++i)
		{
			vertices[i].position = glm::vec3(x, y, z);
			vertices[i].color = gradientColors[cIdx];//gradientColors[i % gradientColors.size()];

			if (i % 4 == 0 && i != 0)
				cIdx++;

			size_t iMod = i % 4;
			if (iMod == 0)
				x = -x; 
			else if (iMod == 1)
				y = -y; 
			else if (iMod == 2)
				x = -x; 
			else
			{
				z = -z; 
				y = -y;
			}
		}

		uint32_t leftUp = 0, rightUp = 1, rightDown = 2, leftDown = 3; 
		for (size_t i = 0; i < 6; ++i)
		{
			indices[i * 6L] = leftUp;
			indices[i * 6L + 1] = rightUp;
			indices[i * 6L + 2] = rightDown;
			indices[i * 6L + 3] = rightDown;
			indices[i * 6L + 4] = leftDown;
			indices[i * 6L + 5] = leftUp;

			if (i == 0)
				leftUp = 1, rightUp = 5, rightDown = 6, leftDown = 2;
			else if (i == 1)
				leftUp = 5, rightUp = 3, rightDown = 7, leftDown = 6;
			else if (i == 2)
				leftUp = 3, rightUp = 0, rightDown = 3, leftDown = 7;
			else if (i == 4)
				leftUp = 3, rightUp = 4, rightDown = 1, leftDown = 0;
			else
				leftUp = 7, rightUp = 6, rightDown = 2, leftDown = 3;
		}
		
		std::vector<Vertex> _vertices{
 
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
 
      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
 
      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
 
      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
 
      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
 
      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
 
  };
		//std::vector<Vertex>* vtemp = new std::vector<Vertex>(vertices.begin(), vertices.begin() + 4);
		//std::vector<uint32_t>* itemp = new std::vector<uint32_t>(indices.begin(), indices.begin() + 6);
	
		Cube rectangle(device, _vertices, indices, commandPool);
		rectangle.transform.position = position;
		rectangle.dimensions = dimensions;
		rectangle.color = color;
		return rectangle;
	}
}
