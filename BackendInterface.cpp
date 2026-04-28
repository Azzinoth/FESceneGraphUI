#include "BackendInterface.h"
using namespace SceneGraphUI;

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

std::string NodeHandle::GetID() const
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

std::string BackendInterface::GetUniqueID()
{
	static std::random_device RandomDevice;
	static std::mt19937 RandomEngine(RandomDevice());
	static std::uniform_int_distribution<int> Distribution(0, 128);

	static bool FirstInitialization = true;
	if (FirstInitialization)
	{
		srand(static_cast<unsigned>(time(nullptr)));
		FirstInitialization = false;
	}

	std::string ID;
	ID += static_cast<char>(Distribution(RandomEngine));
	for (size_t j = 0; j < 11; j++)
	{
		ID.insert(rand() % ID.size(), 1, static_cast<char>(Distribution(RandomEngine)));
	}

	return ID;
}

std::string BackendInterface::GetUniqueHexID()
{
	const std::string ID = GetUniqueID();
	std::string IDinHex;

	for (size_t i = 0; i < ID.size(); i++)
	{
		IDinHex.push_back("0123456789ABCDEF"[(ID[i] >> 4) & 15]);
		IDinHex.push_back("0123456789ABCDEF"[ID[i] & 15]);
	}

	const std::string AdditionalRandomness = GetUniqueID();
	std::string AdditionalString;
	for (size_t i = 0; i < ID.size(); i++)
	{
		AdditionalString.push_back("0123456789ABCDEF"[(AdditionalRandomness[i] >> 4) & 15]);
		AdditionalString.push_back("0123456789ABCDEF"[AdditionalRandomness[i] & 15]);
	}
	std::string FinalID;

	for (size_t i = 0; i < ID.size() * 2; i++)
	{
		if (rand() % 2 - 1)
		{
			FinalID += IDinHex[i];
		}
		else
		{
			FinalID += AdditionalString[i];
		}
	}

	return FinalID;
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