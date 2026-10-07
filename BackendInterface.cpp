#include "BackendInterface.h"
#include <algorithm>
#include <array>
#include <mutex>
using namespace SceneGraphUI;

FEUUID SceneGraphUI::GenerateID()
{
	static std::mutex IDGenerationMutex;
	static std::mt19937 RandomEngine = []() {
		std::random_device RandomDevice;
		std::array<unsigned int, std::mt19937::state_size> SeedData;
		std::generate(SeedData.begin(), SeedData.end(), std::ref(RandomDevice));
		std::seed_seq Sequence(SeedData.begin(), SeedData.end());
		return std::mt19937(Sequence);
	}();
	static uuids::uuid_random_generator Generator(RandomEngine);

	std::lock_guard<std::mutex> Lock(IDGenerationMutex);
	return Generator();
}

FEUUID SceneGraphUI::ConvertLegacyHexID(const std::string& HexID)
{
	// Fixed namespace for converting old hex IDs, must never change.
	// Must be the same as in FEBasicApplication, so the same legacy ID gives the same FEUUID in every module.
	static const FEUUID LegacyIDNamespace = uuids::uuid::from_string("040bcbf8-3a7c-4815-9da7-117bfb3f9bde").value();
	uuids::uuid_name_generator NameGenerator(LegacyIDNamespace);
	return NameGenerator(HexID);
}

bool SceneGraphUI::IsNull(const FEUUID& ID)
{
	return ID.is_nil();
}

std::string SceneGraphUI::ToString(const FEUUID& ID)
{
	return uuids::to_string(ID);
}

FEUUID SceneGraphUI::FromString(const std::string& ID)
{
	auto Result = uuids::uuid::from_string(ID);
	if (!Result.has_value())
		return FEUUID();

	return Result.value();
}

FEUUID SceneGraphUI::FromStringLegacyCompatible(const std::string& ID)
{
	if (ID.empty())
		return FEUUID();

	auto Result = uuids::uuid::from_string(ID);
	if (Result.has_value())
		return Result.value();

	return ConvertLegacyHexID(ID);
}

NodeHandle::NodeHandle(void* InNode, BackendInterface* InBackend) : Node(InNode), Backend(InBackend)
{
	if (Node != nullptr && Backend != nullptr)
		NodeID = Backend->GetNodeID(*this);
}

void* NodeHandle::Raw() const
{
	return Node;
}

BackendInterface* NodeHandle::GetBackend() const
{
	return Backend;
}

bool NodeHandle::WasInitialized() const
{
	return Node != nullptr && Backend != nullptr;
}

FEUUID NodeHandle::GetID() const
{
	return NodeID;
}

std::string NodeHandle::GetName() const
{
	return Backend->GetNodeName(*this);
}

NodeHandle NodeHandle::GetParent() const
{
	return Backend->GetParent(*this);
}

std::vector<NodeHandle> NodeHandle::GetChildren() const
{
	return Backend->GetChildren(*this);
}

size_t NodeHandle::GetDepth() const
{
	return Backend->GetDepth(*this);
}

std::string NodeHandle::GetTag() const
{
	return Backend->GetTag(*this);
}

std::string BackendInterface::TruncateText(const std::string& Text, float MaxWidth, EllipsisPosition Position, const std::string& Ellipsis)
{
	if (Text.empty())
		return Text;

	float TextWidth = ImGui::CalcTextSize(Text.c_str()).x;
	if (TextWidth <= MaxWidth)
		return Text;

	float EllipsisWidth = ImGui::CalcTextSize(Ellipsis.c_str()).x;
	if (MaxWidth <= EllipsisWidth)
		return Ellipsis;

	float AvailableWidth = MaxWidth - EllipsisWidth;

	switch (Position)
	{
		case EllipsisPosition::End:
		{
			size_t Low = 0;
			size_t High = Text.size();
			while (Low < High)
			{
				size_t Mid = (Low + High + 1) / 2;
				float Width = ImGui::CalcTextSize(Text.c_str(), Text.c_str() + Mid).x;
				if (Width <= AvailableWidth)
					Low = Mid;
				else
					High = Mid - 1;
			}
			return Text.substr(0, Low) + Ellipsis;
		}

		case EllipsisPosition::Start:
		{
			size_t Low = 0;
			size_t High = Text.size();
			while (Low < High)
			{
				size_t Mid = (Low + High) / 2;
				float Width = ImGui::CalcTextSize(Text.c_str() + Mid).x;
				if (Width <= AvailableWidth)
					High = Mid;
				else
					Low = Mid + 1;
			}
			return Ellipsis + Text.substr(Low);
		}

		case EllipsisPosition::Middle:
		{
			float HalfAvailable = AvailableWidth / 2.0f;

			// Find how much fits from the start
			size_t StartLow = 0;
			size_t StartHigh = Text.size();
			while (StartLow < StartHigh)
			{
				size_t Mid = (StartLow + StartHigh + 1) / 2;
				float Width = ImGui::CalcTextSize(Text.c_str(), Text.c_str() + Mid).x;
				if (Width <= HalfAvailable)
					StartLow = Mid;
				else
					StartHigh = Mid - 1;
			}

			// Find how much fits from the end
			size_t EndLow = 0;
			size_t EndHigh = Text.size();
			while (EndLow < EndHigh)
			{
				size_t Mid = (EndLow + EndHigh) / 2;
				float Width = ImGui::CalcTextSize(Text.c_str() + Mid).x;
				if (Width <= HalfAvailable)
					EndHigh = Mid;
				else
					EndLow = Mid + 1;
			}

			return Text.substr(0, StartLow) + Ellipsis + Text.substr(EndLow);
		}
	}

	return Text;
}

size_t BackendInterface::GetDepth(NodeHandle Node)
{
	size_t Depth = 0;
	NodeHandle Current = GetParent(Node);
	while (Current)
	{
		Depth++;
		Current = GetParent(Current);
	}

	return Depth;
}

std::string BackendInterface::GetTag(NodeHandle)
{
	return {};
}

bool BackendInterface::Rename(NodeHandle, const std::string&)
{
	return false;
}

bool BackendInterface::MoveNode(NodeHandle Node, NodeHandle NewParent)
{
	return false;
}