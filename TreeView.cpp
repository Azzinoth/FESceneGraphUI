#include "TreeView.h"
using namespace SceneGraphUI;
bool TreeView::bLastNodeWasEven = true;

TreeView::TreeView(BackendInterface* Backend)
{
	this->Backend = Backend;
	strcpy_s(FilterBuffer, PlaceholderText.c_str());
}

TreeView::~TreeView() {}

#include "VersionInfo/FE_SCENE_GRAPH_UI_Version.h"
#include "VersionInfo/FEVersionInfo.h"
FE_DEFINE_VERSION_INFO(FE_SCENE_GRAPH_UI_)

std::string TreeView::GetFullVersion()
{
	return "Scene Graph UI " + GetFE_SCENE_GRAPH_UI_VersionInfo().GetFullVersionString();
}

SceneGraphUI::NodeHandle TreeView::GetRenderingRoot() const
{
	return RenderingRoot;
}

void TreeView::SetNodeRenderPredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate)
{
	NodeRenderPredicate = Predicate;
}

void TreeView::SetNodeDisplayNameProvider(std::function<std::string(SceneGraphUI::NodeHandle)> Provider)
{
	NodeDisplayNameProvider = Provider;
}

void TreeView::SetNodeChildrenVisiblePredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate)
{
	NodeChildrenVisiblePredicate = Predicate;
}

void TreeView::SetNodeIconProvider(std::function<ImTextureID(NodeHandle)> Provider)
{
	NodeIconProvider = Provider;
}

void TreeView::SetNodeSelectionPredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate)
{
	NodeSelectionPredicate = Predicate;
}

void TreeView::ClearAllProvidersAndPredicates()
{
	NodeRenderPredicate = nullptr;
	NodeDisplayNameProvider = nullptr;
	NodeChildrenVisiblePredicate = nullptr;
	NodeIconProvider = nullptr;
	NodeSelectionPredicate = nullptr;
}

ImTextureID TreeView::GetNodeIcon(SceneGraphUI::NodeHandle Node)
{
	if (NodeIconProvider != nullptr)
		return NodeIconProvider(Node);

	return ImTextureID();
}

std::string TreeView::GetNodeDisplayName(SceneGraphUI::NodeHandle Node)
{
	std::string DisplayedName = Node.GetName();
	if (NodeDisplayNameProvider != nullptr)
		DisplayedName = NodeDisplayNameProvider(Node);

	return DisplayedName;
}

std::vector<std::string> TreeView::GetHiddenEntityTags() const
{
	return HiddenEntityTags;
}

void TreeView::SetHiddenEntityTags(const std::vector<std::string>& NewHiddenEntityTags)
{
	HiddenEntityTags = NewHiddenEntityTags;
}

void TreeView::AddHiddenEntityTag(const std::string& TagToAdd)
{
	if (std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), TagToAdd) == HiddenEntityTags.end())
		HiddenEntityTags.push_back(TagToAdd);
}

void TreeView::RemoveHiddenEntityTag(const std::string& TagToRemove)
{
	auto Iterator = std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), TagToRemove);
	if (Iterator != HiddenEntityTags.end())
		HiddenEntityTags.erase(Iterator);
}

void TreeView::ClearHiddenEntityTags()
{
	HiddenEntityTags.clear();
}

bool TreeView::DoesNodePassTextFilter(SceneGraphUI::NodeHandle Node)
{
	if (!Node)
		return false;

	if (!bFilterEnabled)
		return true;

	if (FilterText.empty())
		return true;

	std::string UsedFilterText = FilterText;
	std::string NodeDisplayName = GetNodeDisplayName(Node);
	if (!bCaseSensitiveFiltering)
	{
		std::transform(NodeDisplayName.begin(), NodeDisplayName.end(), NodeDisplayName.begin(), ::tolower);
		std::transform(UsedFilterText.begin(), UsedFilterText.end(), UsedFilterText.begin(), ::tolower);
	}

	if (NodeDisplayName.find(UsedFilterText) != std::string::npos)
		return true;

	// Check all children recursively, if at least one of them passes the filter, then this node should be visible as well.
	for (NodeHandle Child : Node.GetChildren())
	{
		if (DoesNodePassTextFilter(Child))
			return true;
	}

	return false;
}

bool TreeView::ShouldNodeBeVisible(NodeHandle Node)
{
	if (!Node)
		return false;

	std::string NodeTag = Node.GetTag();
	if (std::find(HiddenEntityTags.begin(), HiddenEntityTags.end(), NodeTag) != HiddenEntityTags.end())
		return false;

	if (!DoesNodePassTextFilter(Node))
		return false;
	
	if (NodeRenderPredicate != nullptr)
		return NodeRenderPredicate(Node);
	
	return true;
}

bool TreeView::AreNodeChildrenVisible(NodeHandle Node)
{
	if (!Node)
		return false;

	if (Node.GetChildren().size() == 0)
		return false;

	if (NodeChildrenVisiblePredicate != nullptr)
		return NodeChildrenVisiblePredicate(Node);

	return true;
}

bool TreeView::IsNodeExpanded(SceneGraphUI::NodeHandle Node)
{
	std::string NodeID = Node.GetID();
	bool bResult = false;
	if (NodeState.find(NodeID) == NodeState.end())
	{
		NodeState[NodeID].bExpanded = false;
	}
	else
	{
		bResult = NodeState[NodeID].bExpanded;
	}

	return bResult;
}

void TreeView::SetNodeExpanded(SceneGraphUI::NodeHandle Node, bool bExpanded)
{
	NodeState[Node.GetID()].bExpanded = bExpanded;
}

void TreeView::ExpandToNode(SceneGraphUI::NodeHandle Node)
{
	SceneGraphUI::NodeHandle Current = Node.GetParent();
	while (Current)
	{
		NodeState[Current.GetID()].bExpanded = true;
		Current = Current.GetParent();
	}
}

bool TreeView::IsNodeExpandedTo(SceneGraphUI::NodeHandle Node)
{
	SceneGraphUI::NodeHandle Current = Node.GetParent();
	while (Current)
	{
		if (!NodeState[Current.GetID()].bExpanded)
			return false;

		Current = Current.GetParent();
	}

	return true;
}

bool TreeView::IsNodeSelected(SceneGraphUI::NodeHandle Node)
{
	bool bResult = false;
	std::string NodeID = Node.GetID();
	if (NodeSelectionPredicate != nullptr)
	{
		bool bResult = NodeSelectionPredicate(Node);
		SetNodeSelectedInternal(Node, bResult);

		return bResult;
	}

	if (NodeState.find(NodeID) == NodeState.end())
	{
		SetNodeSelectedInternal(Node, false);
	}
	else
	{
		bResult = NodeState[NodeID].bSelected;
	}

	return bResult;
}

void TreeView::SetNodeSelectedInternal(SceneGraphUI::NodeHandle Node, bool bSelected)
{
	if (!Node)
		return;

	if (NodeState[Node.GetID()].bSelected == bSelected)
		return;

	if (bSelected)
		ExpandToNode(Node);

	bool bOldSelectionState = NodeState[Node.GetID()].bSelected;
	NodeState[Node.GetID()].bSelected = bSelected;

	for (const auto& Callback : OnNodeSelectionChangedCallbacks)
		Callback(Node, bOldSelectionState);
}

void TreeView::SetNodeSelected(NodeHandle Node, bool bSelected)
{
	if (!bAllowMultipleNodeSelection && bSelected)
	{
		for (auto& NodeStatePair : NodeState)
		{
			if (NodeStatePair.first == Node.GetID())
				continue;

			NodeHandle CurrentNode = Backend->GetNodeByID(NodeStatePair.first);
			// Node might be null if it has been deleted but its state has not been cleaned up yet.
			// FE_FIX_ME: Clean up state of deleted nodes to avoid this situation.
			if (Backend->IsAlive(CurrentNode))
				SetNodeSelectedInternal(CurrentNode, false);
		}
	}

	SetNodeSelectedInternal(Node, bSelected);
}

std::vector<std::string> TreeView::GetSelectedNodeIDs() const
{
	std::vector<std::string> SelectedNodeIDs;
	for (const auto& NodeStatePair : NodeState)
	{
		if (NodeStatePair.second.bSelected)
			SelectedNodeIDs.push_back(NodeStatePair.first);
	}

	return SelectedNodeIDs;
}

bool TreeView::IsNodePartOfBranch(SceneGraphUI::NodeHandle NodeToCheck, SceneGraphUI::NodeHandle BranchRoot, SceneGraphUI::NodeHandle BranchLeaf)
{
	if (!NodeToCheck || !BranchRoot || !BranchLeaf)
		return false;

	if (NodeToCheck == BranchRoot || NodeToCheck == BranchLeaf)
		return true;

	// Walk from BranchLeaf up to BranchRoot.
	// NodeToCheck is part of the branch only if it lies on this path.
	NodeHandle CurrentNode = BranchLeaf;
	while (CurrentNode)
	{
		if (CurrentNode == NodeToCheck)
			return true;

		// Stop once we have reached the root of the branch.
		if (CurrentNode == BranchRoot)
			break;

		CurrentNode = CurrentNode.GetParent();
	}

	return false;
}

void TreeView::DrawTreeConnectorLines(SceneGraphUI::NodeHandle Node, float ParentBottomY)
{
	ImColor ConnectorLineColorToUse = ImColor(this->ConnectorLineColor);
	float ConnectorLineThicknessToUse = ConnectorLineThickness;
	bool bNeedToHighlightNodeBranch = false;
	std::vector<std::string> SelectedNodeIDs = GetSelectedNodeIDs();
	if (bHighlightSelectedNodeConnectorLines && !SelectedNodeIDs.empty())
	{
		for (size_t i = 0; i < SelectedNodeIDs.size(); i++)
		{
			if (IsNodePartOfBranch(Node, RenderingRoot, Backend->GetNodeByID(SelectedNodeIDs[i])))
			{
				ConnectorLineColorToUse = SelectedNodeConnectorLineColor;
				ConnectorLineThicknessToUse = SelectedConnectorLineThickness;
				bNeedToHighlightNodeBranch = true;
			}
		}
	}
	ImGui::GetWindowDrawList()->ChannelsSetCurrent(bNeedToHighlightNodeBranch ? 1 : 0);

	size_t Depth = Node.GetDepth();
	Depth -= RenderingRoot.GetDepth() + (bShowRoot ? 0 : 1);

	float BaseX = ImGui::GetCursorScreenPos().x;

	int HorizontalOffset = static_cast<int>((Depth - 1) * NodeHeight);
	ImVec2 VerticalStart = ImVec2(BaseX + HorizontalOffset + NodeHeight / 2.0f,
		ParentBottomY + NodeHeight);

	ImVec2 ElbowPoint = ImVec2(VerticalStart.x,
		ImGui::GetCursorScreenPos().y + NodeHeight / 2.0f);

	ImGui::GetWindowDrawList()->AddLine(VerticalStart, ElbowPoint, ImColor(ConnectorLineColorToUse), ConnectorLineThicknessToUse);

	bool bHasChildren = AreNodeChildrenVisible(Node);
	ImVec2 HorizontalEnd = ImVec2(ElbowPoint.x + NodeHeight / (bHasChildren ? 2.0f : 0.7f),
		ElbowPoint.y);

	ImGui::GetWindowDrawList()->AddLine(ElbowPoint, HorizontalEnd, ImColor(ConnectorLineColorToUse), ConnectorLineThicknessToUse);

	if (bNeedToHighlightNodeBranch)
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
}

void TreeView::DrawAppropriateTreeArrow(SceneGraphUI::NodeHandle Node)
{
	float ArrowRegionWidth = FontSize;
	ImVec2 ArrowCursorPos = ImGui::GetCursorScreenPos();

	bool bNodeExpanded = IsNodeExpanded(Node);
	bool bHasChildren = AreNodeChildrenVisible(Node);

	if (bHasChildren)
	{
		// Draw arrow centered within the reserved region
		float ArrowHalfSize = FontSize * 0.25f;
		float Thickness = FontSize * TreeArrowsThicknessCoefficient;
		ImU32 Color = ImGui::GetColorU32(ImGuiCol_Text);
		float CenterX = ArrowCursorPos.x + ArrowRegionWidth * 0.5f;
		float CenterY = ArrowCursorPos.y + NodeHeight * 0.5f;

		if (bNodeExpanded)
		{
			// Down-pointing arrow (collapse)
			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX - ArrowHalfSize, CenterY - ArrowHalfSize * 0.3f),
				ImVec2(CenterX, CenterY + ArrowHalfSize * LineJoinOverlapFactor),
				Color, Thickness);

			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX, CenterY + ArrowHalfSize * 0.6f),
				ImVec2(CenterX + ArrowHalfSize, CenterY - ArrowHalfSize * 0.3f),
				Color, Thickness);
		}
		else
		{
			// Right-pointing arrow (expand)
			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX - ArrowHalfSize * 0.3f, CenterY - ArrowHalfSize),
				ImVec2(CenterX + ArrowHalfSize * LineJoinOverlapFactor, CenterY),
				Color, Thickness);

			ImGui::GetWindowDrawList()->AddLine(ImVec2(CenterX + ArrowHalfSize * 0.6f, CenterY),
				ImVec2(CenterX - ArrowHalfSize * 0.3f, CenterY + ArrowHalfSize),
				Color, Thickness);
		}

		// Occupy the space in ImGui layout.
		ImGui::InvisibleButton(("##Arrow" + Node.GetID()).c_str(), ImVec2(ArrowRegionWidth, NodeHeight));
		if (ImGui::IsItemClicked())
			SetNodeExpanded(Node, !bNodeExpanded);
	}
	else
	{
		ImGui::Dummy(ImVec2(ArrowRegionWidth, NodeHeight));
	}

	ImGui::SameLine();
}

void TreeView::CheckInputs(NodeHandle Node)
{
	if (NodeIDBeingRenamed == Node.GetID())
		return;

	if (ImGui::IsItemHovered())
	{
		HoveredNodeID = Node.GetID();

		for (auto& Callback : OnNodeHoveredCallbacks)
			Callback(Node);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			SetNodeSelected(Node, !IsNodeSelected(Node));

			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Left);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Right);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Middle))
		{
			for (auto& Callback : OnNodeClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Middle);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Left);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Right);
		}

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Middle))
		{
			for (auto& Callback : OnNodeDoubleClickedCallbacks)
				Callback(Node, ImGuiMouseButton_Middle);
		}
	}
}

void TreeView::AddOnNodeClickedCallback(std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)> Callback)
{
	for (size_t i = 0; i < OnNodeClickedCallbacks.size(); i++)
	{
		if (OnNodeClickedCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeClickedCallbacks.push_back(Callback);
}

void TreeView::ClearOnNodeClickedCallbacks()
{
	OnNodeClickedCallbacks.clear();
}

void TreeView::AddOnNodeDoubleClickedCallback(std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)> Callback)
{
	for (size_t i = 0; i < OnNodeDoubleClickedCallbacks.size(); i++)
	{
		if (OnNodeDoubleClickedCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeDoubleClickedCallbacks.push_back(Callback);
}

void TreeView::ClearOnNodeDoubleClickedCallbacks()
{
	OnNodeDoubleClickedCallbacks.clear();
}

void TreeView::AddOnNodeSelectionChangedCallback(std::function<void(SceneGraphUI::NodeHandle, bool)> Callback)
{
	for (size_t i = 0; i < OnNodeSelectionChangedCallbacks.size(); i++)
	{
		if (OnNodeSelectionChangedCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeSelectionChangedCallbacks.push_back(Callback);
}

void TreeView::ClearOnNodeSelectionChangedCallbacks()
{
	OnNodeSelectionChangedCallbacks.clear();
}

void TreeView::ClearAllInputCallbacks()
{
	ClearOnNodeHoveredCallbacks();
	ClearOnNodeClickedCallbacks();
	ClearOnNodeDoubleClickedCallbacks();
	ClearOnNodeSelectionChangedCallbacks();
}

void TreeView::SetContextMenuRenderingFunction(std::function<void(SceneGraphUI::NodeHandle)> Function)
{
	ContextMenuRenderer = Function;
}

void TreeView::ClearContextMenuRenderingFunction()
{
	ContextMenuRenderer = nullptr;
}

void TreeView::ClearAllCallbacks()
{
	ClearAllProvidersAndPredicates();
	ClearAllInputCallbacks();

	BeforeNodeRenderCallbacks.clear();
	AfterNodeRenderCallbacks.clear();
	ClearContextMenuRenderingFunction();
	ClearRenameNodeFunction();
}

void TreeView::AddOnNodeHoveredCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback)
{
	for (size_t i = 0; i < OnNodeHoveredCallbacks.size(); i++)
	{
		if (OnNodeHoveredCallbacks[i].target_type() == Callback.target_type())
			return;
	}

	OnNodeHoveredCallbacks.push_back(Callback);
}

void TreeView::ClearOnNodeHoveredCallbacks()
{
	OnNodeHoveredCallbacks.clear();
}

NodeWidget* TreeView::GetNodeWidgetByID(const std::string& WidgetID)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == WidgetID)
			return &NodeWidgets[i];
	}

	return nullptr;
}

std::vector<NodeWidget> TreeView::GetAllNodeWidgets() const
{
	return NodeWidgets;
}

bool TreeView::AddNodeWidget(NodeWidget& Widget)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == Widget.GetID())
			return false;
	}

	NodeWidgets.push_back(Widget);
	return true;
}

bool TreeView::RemoveNodeWidget(const std::string& WidgetID)
{
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		if (NodeWidgets[i].GetID() == WidgetID)
		{
			NodeWidgets.erase(NodeWidgets.begin() + i);
			return true;
		}
	}

	return false;
}

void TreeView::ClearNodeWidgets()
{
	NodeWidgets.clear();
	//DebugNodeWidgets.clear();
}

bool TreeView::ShouldRenderWidgetForNode(SceneGraphUI::NodeHandle Node, NodeWidget& Widget, ImTextureID& IconToUse)
{
	bool bVisible = Widget.bIsVisibleByDefault;
	if (Widget.IsVisiblePredicate != nullptr)
		bVisible = Widget.IsVisiblePredicate(Node);

	if (!bVisible)
		return false;

	ImTextureID ResultingIcon = Widget.Icon;
	if (Widget.DynamicIconProvider != nullptr)
	{
		ImTextureID ProviderIcon = Widget.DynamicIconProvider(Node);
		if (ProviderIcon != 0)
			ResultingIcon = ProviderIcon;
	}

	if (ResultingIcon == 0)
		return false;

	IconToUse = ResultingIcon;
	return true;
}

float TreeView::GetNodeWidgetAreaWidth(SceneGraphUI::NodeHandle Node)
{
	float IconSpacing = GetFontSize() * 0.15f;

	int VisibleWidgets = static_cast<int>(GetNodeWidgetCount(Node));
	float SpaceNeededForIconsAtEnd = IconsSize.x * WidgetIconScale * VisibleWidgets + IconSpacing * std::max(0, VisibleWidgets - 1);
	return SpaceNeededForIconsAtEnd + ImGui::GetStyle().WindowPadding.x + 6.0f;
}

size_t TreeView::GetNodeWidgetCount(SceneGraphUI::NodeHandle Node)
{
	int VisibleWidgets = 0;
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		NodeWidget& Widget = NodeWidgets[i];

		ImTextureID NotUsedTextureID;
		if (!ShouldRenderWidgetForNode(Node, Widget, NotUsedTextureID))
			continue;

		VisibleWidgets++;
	}

	return VisibleWidgets;
}

void TreeView::RenderNodeWidgets(SceneGraphUI::NodeHandle Node)
{
	YCursorPositionBeforeRenderingWidgets = ImGui::GetCursorPosY();
	float IconSpacing = GetFontSize() * 0.15f;

	size_t VisibleWidgetCount = GetNodeWidgetCount(Node);
	size_t WidgetIndex = 0;
	for (size_t i = 0; i < NodeWidgets.size(); i++)
	{
		NodeWidget& Widget = NodeWidgets[i];

		ImTextureID IconToUse = 0;
		if (!ShouldRenderWidgetForNode(Node, Widget, IconToUse))
			continue;

		// We change item spacing only for the widgets that are not the first one.
		if (WidgetIndex == 1)
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(IconSpacing, 0));

		ImGui::SameLine();
		// Because we render icons a little bit smaller(using WidgetIconVisualRenderingFactor), we need to adjust the position of the icons to make them look vertically centered.
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (IconsSize.y * (1.0f - WidgetIconScale)) / 2.0f);

		if (Widget.bIsInteractive)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Widget.HoveredColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, Widget.ActiveColor);
			
			std::string NodeID = Node.GetID();
			std::string ButtonID = "##" + Widget.ID + "_" + NodeID;
			if (ImGui::ImageButton(ButtonID.c_str(), IconToUse, IconsSize * WidgetIconScale))
			{
				if (Widget.OnClickCallback != nullptr)
					Widget.OnClickCallback(Node);

				// Ensure the node is still alive after the callback, as it might have been deleted.
				Backend->IsAlive(Node); 
				if (!Backend->IsAlive(Node))
				{
					ImGui::PopStyleVar();
					ImGui::PopStyleColor(3);
					break;
				}
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);
		}
		else
		{
			ImGui::Image(IconToUse, IconsSize * WidgetIconScale);
		}

		if (ImGui::IsItemHovered() && !Widget.TooltipText.empty())
		{
			if (DefaultFont != nullptr)
				ImGui::PushFont(DefaultFont, TooltipFontSize);

			ImGui::BeginTooltip();
			ImGui::TextUnformatted(Widget.TooltipText.c_str());
			ImGui::EndTooltip();

			if (DefaultFont != nullptr)
				ImGui::PopFont();
		}

		WidgetIndex++;
	}

	if (WidgetIndex > 1)
		ImGui::PopStyleVar();

	// Node widgets render on the same line as the Selectable and shift the cursor Y position due to vertical centering adjustments.
	// We restore the cursor to the pre-widget Y position so the next node starts at the correct vertical offset.
	// After all nodes are rendered, we set the cursor to correct position to ensure the parent container (e.g. ListBox) accounts for the full content height.
	// If not accounted for, ImGui will assert with request to use Dummy().
	YCursorPositionAfterRenderingWidgets = ImGui::GetCursorPosY();
	if (YCursorPositionBeforeRenderingWidgets != YCursorPositionAfterRenderingWidgets)
		ImGui::SetCursorPosY(YCursorPositionBeforeRenderingWidgets);
}

//void FESceneGraphUI::RenderNodeWidgets(FENaiveSceneGraphNode* Node)
//{
//	YCursorPositionBeforeRenderingWidgets = ImGui::GetCursorPosY();
//	float IconSpacing = GetFontSize() * 0.15f;
//
//	size_t VisibleWidgetCount = GetNodeWidgetCount(Node);
//	size_t WidgetIndex = 0;
//	for (size_t i = 0; i < NodeWidgets.size(); i++)
//	{
//		FESceneGraphNodeWidget& Widget = NodeWidgets[i];
//
//		FETexture* IconToUse = nullptr;
//		if (!ShouldRenderWidgetForNode(Node, Widget, &IconToUse))
//			continue;
//
//		// We change item spacing only for the widgets that are not the first one.
//		if (WidgetIndex == 1)
//			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(IconSpacing, 0));
//
//		ImGui::SameLine();
//		// Because we render icons a little bit smaller(using WidgetIconVisualRenderingFactor), we need to adjust the position of the icons to make them look vertically centered.
//		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (IconsSize.y * (1.0f - WidgetIconVisualRenderingFactor)) / 2.0f);
//
//		if (Widget.bIsInteractive)
//		{
//			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
//			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
//			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Widget.HoveredColor);
//			ImGui::PushStyleColor(ImGuiCol_ButtonActive, Widget.ActiveColor);
//
//			std::string ButtonID = "##" + Widget.ID + "_" + Node->GetObjectID();
//			if (ImGui::ImageButton(ButtonID.c_str(), IconToUse->GetTextureID(), IconsSize * WidgetIconVisualRenderingFactor))
//			{
//				std::string NodeID = Node->GetObjectID();
//				if (Widget.OnClickCallback != nullptr)
//					Widget.OnClickCallback(Node);
//
//				// After these callbacks, the node might not be valid anymore (e.g. it could be removed in the callback).
//				FENaiveSceneGraphNode* NodeAfterCallbacks = GetScene()->SceneGraph.GetNodeByID(NodeID);
//				if (NodeAfterCallbacks == nullptr)
//				{
//					ImGui::PopStyleVar();
//					ImGui::PopStyleColor(3);
//					break;
//				}
//			}
//
//			ImGui::PopStyleVar();
//			ImGui::PopStyleColor(3);
//		}
//		else
//		{
//			ImGui::Image(IconToUse->GetTextureID(), IconsSize * WidgetIconVisualRenderingFactor);
//		}
//
//		if (ImGui::IsItemHovered() && !Widget.TooltipText.empty())
//		{
//			if (CousineFont != nullptr)
//				ImGui::PushFont(CousineFont, TooltipFontSize);
//
//			ImGui::BeginTooltip();
//			ImGui::TextUnformatted(Widget.TooltipText.c_str());
//			ImGui::EndTooltip();
//
//			if (CousineFont != nullptr)
//				ImGui::PopFont();
//		}
//
//		WidgetIndex++;
//	}
//
//	if (WidgetIndex > 1)
//		ImGui::PopStyleVar();
//
//	// Node widgets render on the same line as the Selectable and shift the cursor Y position due to vertical centering adjustments.
//	// We restore the cursor to the pre-widget Y position so the next node starts at the correct vertical offset.
//	// After all nodes are rendered, we set the cursor to correct position to ensure the parent container (e.g. ListBox) accounts for the full content height.
//	// If not accounted for, ImGui will assert with request to use Dummy().
//	YCursorPositionAfterRenderingWidgets = ImGui::GetCursorPosY();
//	if (YCursorPositionBeforeRenderingWidgets != YCursorPositionAfterRenderingWidgets)
//		ImGui::SetCursorPosY(YCursorPositionBeforeRenderingWidgets);
//}

void TreeView::RenderNode(NodeHandle Node)
{
	if (!ShouldNodeBeVisible(Node))
		return;

	DrawAppropriateTreeArrow(Node);

	ImTextureID BeforeNodeIcon = GetNodeIcon(Node);
	if (BeforeNodeIcon != 0)
	{
		ImGui::Image(BeforeNodeIcon, IconsSize);
		ImGui::SameLine();
	}

	float IconSpacing = GetFontSize() * 0.15f;
	float SpaceNeededForWidgetAtEnd = GetNodeWidgetAreaWidth(Node);
	float NodeBodyWidth = ImGui::GetContentRegionAvail().x - SpaceNeededForWidgetAtEnd - IconSpacing;

	std::string DisplayedName = GetNodeDisplayName(Node);
	std::string DisplayedText = BackendInterface::TruncateText(DisplayedName, NodeBodyWidth) + "##" + Node.GetID();

	if (bAlternatingNodeBackground)
	{
		bLastNodeWasEven = !bLastNodeWasEven;
		ImVec2 RectMin = ImGui::GetCursorScreenPos();
		ImVec2 RectMax = ImVec2(RectMin.x + NodeBodyWidth, RectMin.y + NodeHeight);
		ImGui::GetWindowDrawList()->AddRectFilled(RectMin, RectMax, bLastNodeWasEven ? ImColor(EvenNodeBackgroundColor) : ImColor(OddNodeBackgroundColor));
	}

	for (size_t i = 0; i < BeforeNodeRenderCallbacks.size(); i++)
		BeforeNodeRenderCallbacks[i](Node);

	bool bIsSelected = IsNodeSelected(Node);
	if (NodeIDBeingRenamed == Node.GetID())
	{
		if (!bLastFrameRenameEditWasVisible)
		{
			ImGui::SetKeyboardFocusHere(0);

			ImGuiContext* Context = ImGui::GetCurrentContext();
			if (Context != nullptr)
				ImGui::SetFocusID(ImGui::GetID("##SceneGraphRenameEditor"), Context->CurrentWindow);

			ImGui::SetItemDefaultFocus();
			bLastFrameRenameEditWasVisible = true;
		}

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

		ImGui::SetNextItemWidth(NodeBodyWidth);
		if (ImGui::InputText("##SceneGraphRenameEditor", RenameBuffer, IM_ARRAYSIZE(RenameBuffer), ImGuiInputTextFlags_EnterReturnsTrue) ||
			ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || !ImGui::IsItemFocused())
		{
			if (RenameNodeFunction != nullptr)
				RenameNodeFunction(Node, RenameBuffer);
			/*FEEntity* ObjectToRename = Node->GetEntity();
			if (ObjectToRename != nullptr)
				ObjectToRename->SetName(RenameBuffer);*/

			NodeIDBeingRenamed = "";
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}
	else
	{
		ImGui::Selectable(DisplayedText.c_str(), bIsSelected, ImGuiSelectableFlags_None, ImVec2(NodeBodyWidth, NodeHeight));
	}

	for (size_t i = 0; i < AfterNodeRenderCallbacks.size(); i++)
		AfterNodeRenderCallbacks[i](Node);

	CheckInputs(Node);
	std::string NodeID = Node.GetID();
	RenderNodeWidgets(Node);

	// Ensure the node is still alive after the callbacks, as it might have been deleted.
	if (!Backend->IsAlive(Node))
		return;

	if (IsNodeExpanded(Node))
	{
		std::vector<SceneGraphUI::NodeHandle> Children = Node.GetChildren();
		float ParentBottomY = ImGui::GetItemRectMin().y;
		for (size_t i = 0; i < Children.size(); i++)
		{
			if (ShouldNodeBeVisible(Children[i]))
			{
				DrawTreeConnectorLines(Children[i], ParentBottomY);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (Children[i].GetDepth() - RenderingRoot.GetDepth() - (bShowRoot ? 0 : 1)) * NodeHeight);
				RenderNode(Children[i]);
			}
		}
	}
}

float TreeView::GetFontSize() const
{
	return FontSize;
}

void TreeView::SetFontSize(float NewFontSize)
{
	FontSize = NewFontSize;
	TooltipFontSize = std::max(FontSize / 2.0f, 16.0f);
	NodeHeight = FontSize;
	IconsSize = ImVec2(FontSize, FontSize);
}

//void FESceneGraphUI::DebugCreateRandomWidgets(bool bInteractive)
//{
//	FETexture* RandomIcon = GetRandomDebugIcon();
//	FESceneGraphNodeWidget NewDebugWidget;
//	NewDebugWidget.Icon = RandomIcon;
//	NewDebugWidget.bIsInteractive = bInteractive;
//
//	NewDebugWidget.IsVisiblePredicate = [WidgetID = NewDebugWidget.GetID()](FENaiveSceneGraphNode* Node) -> bool {
//		FEEntity* CurrentEntity = Node->GetEntity();
//		if (CurrentEntity == nullptr)
//			return true;
//
//		size_t Seed = std::hash<std::string>{}(CurrentEntity->GetObjectID() + WidgetID);
//		srand(static_cast<unsigned int>(Seed));
//
//		float RandomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//		if (RandomValue < 0.6f)
//			return true;
//		else
//			return false;
//	};
//
//	bool AlreadyExistingWidgetWithSameProperties = false;
//	for (size_t i = 0; i < DebugNodeWidgets.size(); i++)
//	{
//		if (DebugNodeWidgets[i].Icon->GetObjectID() == NewDebugWidget.Icon->GetObjectID() && DebugNodeWidgets[i].bIsInteractive == NewDebugWidget.bIsInteractive)
//			AlreadyExistingWidgetWithSameProperties = true;
//	}
//
//	if (!AlreadyExistingWidgetWithSameProperties)
//	{
//		AddNodeWidget(NewDebugWidget);
//		DebugNodeWidgets.push_back(NewDebugWidget);
//	}
//}

//void FESceneGraphUI::DebugRenderUI()
//{
//	if (ImGui::Button("Expand all"))
//		ExpandAllNodes();
//	
//	if (ImGui::Button("Collapse all"))
//		CollapseAllNodes();
//
//	ImGui::Checkbox("Render root", &bDebugRenderRoot);
//
//	ImGui::Checkbox("Render random icons", &bDebugRenderRandomNodeIcons);
//		
//	if (bDebugRenderRandomNodeIcons)
//	{
//		SetNodeIconProvider([this] (FENaiveSceneGraphNode* Node) -> FETexture* {
//			FEEntity* CurrentEntity = Node->GetEntity();
//			if (CurrentEntity == nullptr)
//				return nullptr;
//
//			// More elegant solution without rand().
//			size_t Hash = std::hash<std::string>{}(CurrentEntity->GetObjectID());
//			size_t IconCount = DebugIconsIDs.size();
//			if (IconCount == 0)
//				return nullptr;
//
//			// +1 to include a "no icon" option.
//			size_t Index = Hash % (IconCount + 1);
//			if (Index >= IconCount)
//				return nullptr;
//
//			return GetDebugIconByIndex(Index);
//		});
//	}
//	else
//	{
//		SetNodeIconProvider(nullptr);
//	}
//
//	if (ImGui::Button("Clear Widgets"))
//		ClearNodeWidgets();
//
//	ImGui::SameLine();
//	if (ImGui::Button("Add interactive widget"))
//		DebugCreateRandomWidgets(true);
//	
//	ImGui::SameLine();
//	if (ImGui::Button("Add non interactive widget"))
//		DebugCreateRandomWidgets(false);
//}

void TreeView::RenderContextMenu()
{
	std::string RootID = RenderingRoot.GetID();

	if (RootID.empty())
		return;

	if (bShouldOpenContextMenu)
	{
		HoveredNodeIDWhenContextMenuWasOpened = HoveredNodeID;
		ImGui::OpenPopup(("##Scene Graph Context Menu " + RootID).c_str());
	}	

	bShouldOpenContextMenu = false;

	if (ImGui::BeginPopup(("##Scene Graph Context Menu " + RootID).c_str()))
	{
		SceneGraphUI::NodeHandle ContextNode = Backend->GetNodeByID(HoveredNodeIDWhenContextMenuWasOpened);
		if (ContextMenuRenderer)
		{
			ContextMenuRenderer(ContextNode);
		}
		else
		{
			ImGui::TextUnformatted("No context menu rendering function set.");
		}
		
		ImGui::EndPopup();
	}
	else
	{
		// Popup was closed (either by user dismissing or item selected).
		HoveredNodeIDWhenContextMenuWasOpened = "";
	}
}

void TreeView::Render(NodeHandle RenderingRoot, bool bRenderRootItself)
{
	if (!bVisible)
		return;

	// If in debug mode ignore the provided inputs.
	//if (IsInDebugMode())
	//{
	//	if (GetTestScene() == nullptr)
	//		return;

	//	RenderingRoot = GetTestScene()->SceneGraph.GetRoot();
	//	bRenderRootItself = bDebugRenderRoot;

	//	DebugRenderUI();
	//}

	if (Backend == nullptr || !Backend->IsReady())
		return;

	if (!RenderingRoot.WasInitialized())
		return;

	//FEScene* CurrentScene = SCENE_MANAGER.GetSceneByNodeID(RenderingRoot->GetObjectID());
	//if (CurrentScene == nullptr)
	//	return;

	//CurrentSceneID = CurrentScene->GetObjectID();
	HoveredNodeID = "";

	this->RenderingRoot = RenderingRoot;
	this->bShowRoot = bRenderRootItself;
	bLastNodeWasEven = false;

	if (DefaultFont == nullptr)
		DefaultFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Resources/Cousine-Regular.ttf", 32.0f);

	if (bRenderTextFilterInput)
		RenderFilterTextInput();

	if (bRenderUIScaleControl)
	{
		if (bRenderTextFilterInput)
			ImGui::SameLine();

		RenderUIScaleControl();
	}

	ImGui::PushStyleColor(ImGuiCol_FrameBg, BackgroundColor);
	if (ImGui::BeginListBox(("##Scene Graph" + RenderingRoot.GetID()).c_str(), ImVec2(ImGui::GetContentRegionAvail())))
	{
		// 0 - default.
		// 1 - highlighted connector lines (e.g. for selected nodes).
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		if (DefaultFont != nullptr)
			ImGui::PushFont(DefaultFont, GetFontSize());

		if (bRenderRootItself)
		{
			RenderNode(RenderingRoot);
		}
		else
		{
			std::vector<NodeHandle> Children = RenderingRoot.GetChildren();
			for (size_t i = 0; i < Children.size(); i++)
			{
				// Rendering each node can trigger callbacks that modify the scene graph,
				// potentially invalidating nodes.
				if (Backend->IsAlive(Children[i]))
					RenderNode(Children[i]);
			}
		}

		// Node widgets render on the same line as the Selectable and shift the cursor Y position due to vertical centering adjustments.
		// We restore the cursor to the pre-widget Y position so the next node starts at the correct vertical offset.
		// After all nodes are rendered, we set the cursor to correct position to ensure the parent container (e.g. ListBox) accounts for the full content height.
		// If not accounted for, ImGui will assert with request to use Dummy().
		if (YCursorPositionBeforeRenderingWidgets > YCursorPositionAfterRenderingWidgets)
			ImGui::SetCursorPosY(YCursorPositionAfterRenderingWidgets);

		if (DefaultFont != nullptr)
			ImGui::PopFont();

		ImGui::EndListBox();
	}
	ImGui::PopStyleColor();

	ImGui::GetWindowDrawList()->ChannelsMerge();

	bWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_ChildWindows);
	if (bWindowHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		bShouldOpenContextMenu = true;

	RenderContextMenu();
}

void TreeView::ExpandAllNodes()
{
	if (!RenderingRoot.WasInitialized() || !Backend->IsAlive(RenderingRoot))
		return;

	std::vector<SceneGraphUI::NodeHandle> Stack;
	Stack.push_back(RenderingRoot);
	while (!Stack.empty())
	{
		SceneGraphUI::NodeHandle CurrentNode = Stack.back();
		Stack.pop_back();
		SetNodeExpanded(CurrentNode, true);
		for (SceneGraphUI::NodeHandle Child : CurrentNode.GetChildren())
			Stack.push_back(Child);
	}
}

void TreeView::CollapseAllNodes()
{
	if (!RenderingRoot.WasInitialized() || !Backend->IsAlive(RenderingRoot))
		return;

	std::vector<SceneGraphUI::NodeHandle> Stack;
	Stack.push_back(RenderingRoot);
	while (!Stack.empty())
	{
		SceneGraphUI::NodeHandle CurrentNode = Stack.back();
		Stack.pop_back();
		SetNodeExpanded(CurrentNode, false);
		for (SceneGraphUI::NodeHandle Child : CurrentNode.GetChildren())
			Stack.push_back(Child);
	}
}

void TreeView::AddBeforeNodeRenderCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback)
{
	BeforeNodeRenderCallbacks.push_back(Callback);
}

void TreeView::AddAfterNodeRenderCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback)
{
	AfterNodeRenderCallbacks.push_back(Callback);
}

void TreeView::RenderUIScaleControl(float Min, float Max)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	float FrameHeight = ImGui::GetFrameHeight();
	float TextHeight = ImGui::GetTextLineHeight();
	float Offset = (FrameHeight - TextHeight) * 0.5f;

	ImGui::Text("UI Scale");
	ImGui::SameLine();
	
	ImGui::SetNextItemWidth(60.0f);
	float LocalFontSize = FontSize;
	ImGui::DragFloat("##FontSize", &LocalFontSize, 0.5f, Min, Max, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	if (LocalFontSize != FontSize)
		SetFontSize(LocalFontSize);
	
	ImGui::PopStyleVar();
}

int TreeView::FilterInputTextCallback(ImGuiInputTextCallbackData* Data)
{
	if (Data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
	{
		bool bIsFocused = ImGui::IsItemActive();
		if (bIsPlaceHolderTextUsed)
		{
			// Check if the input just gained focus
			if (bIsFocused && !bFilterInputWasFocused)
			{
				strcpy_s(FilterBuffer, "");

				// Update ImGui's buffer.
				Data->BufDirty = true;
				Data->DeleteChars(0, Data->BufTextLen);
				Data->InsertChars(0, FilterBuffer);

				bIsPlaceHolderTextUsed = false;
			}
		}

		bFilterInputWasFocused = bIsFocused;
	}

	return 0;
}

void TreeView::RenderFilterTextInput()
{
	const bool bIsPlaceHolderTextUsedWasOn = bIsPlaceHolderTextUsed;

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 150));

	// Setting up the callback for the input text.
	std::function<int(ImGuiInputTextCallbackData*)> Callback = std::bind(&TreeView::FilterInputTextCallback, this, std::placeholders::_1);
	auto StaticCallback = [](ImGuiInputTextCallbackData* Data) -> int {
		const auto& Callback = *static_cast<std::function<int(ImGuiInputTextCallbackData*)>*>(Data->UserData);
		return Callback(Data);
	};

	float AvailableWidth = ImGui::GetContentRegionAvail().x;
	if (bRenderUIScaleControl)
	{
		ImVec2 TextSize = ImGui::CalcTextSize("UI Scale");
		AvailableWidth -= TextSize.x + ImGui::GetStyle().ItemSpacing.x;
		AvailableWidth -= 60.0f + 4.0f; // The width of the DragFloat control.
	}
	
	ImGui::SetNextItemWidth(AvailableWidth);
	if (ImGui::InputText("##SceneGraphWindowFilter", FilterBuffer, FilterInputBufferSize, ImGuiInputTextFlags_CallbackAlways, StaticCallback, &Callback))
	{
		bFilterEnabled = true;
		FilterText = FilterBuffer;
	}

	if (!ImGui::IsItemActive())
	{
		if (strlen(FilterBuffer) == 0)
		{
			strcpy_s(FilterBuffer, PlaceholderText.c_str());
			bIsPlaceHolderTextUsed = true;
			bFilterInputWasFocused = false;
			bFilterEnabled = false;
		}
	}

	if (bIsPlaceHolderTextUsedWasOn)
		ImGui::PopStyleColor();
}

bool TreeView::IsNodeBeingRenamed(SceneGraphUI::NodeHandle Node)
{
	if (!Node.WasInitialized() || !Backend->IsAlive(Node))
		return false;

	return NodeIDBeingRenamed == Node.GetID();
}

bool TreeView::SetNodeForRenaming(SceneGraphUI::NodeHandle Node)
{
	if (!Node.WasInitialized() || !Backend->IsAlive(Node))
		return false;

	if (!ShouldNodeBeVisible(Node))
		return false;

	if (IsNodeBeingRenamed(Node))
		return true;

	if (!IsNodeSelected(Node))
		SetNodeSelected(Node, true);

	NodeIDBeingRenamed = Node.GetID();

	std::string NodeDisplayName = GetNodeDisplayName(Node);
	strcpy_s(RenameBuffer, NodeDisplayName.size() + 1, NodeDisplayName.c_str());
	bLastFrameRenameEditWasVisible = false;
	return true;
}

void TreeView::SetRenameNodeFunction(std::function<void(SceneGraphUI::NodeHandle, std::string)> Function)
{
	RenameNodeFunction = Function;
}

void TreeView::ClearRenameNodeFunction()
{
	RenameNodeFunction = nullptr;
}

bool TreeView::IsInDebugMode()
{
	return bDebugMode;
}

void TreeView::SetDebugMode(bool bNewValue)
{
	bool bModeChanged = bDebugMode != bNewValue;
	bDebugMode = bNewValue;

	if (bModeChanged)
	{
#ifdef SCENE_GRAPH_UI_WITH_FOCAL_ENGINE
		NodeState.clear();

		if (bDebugMode)
		{
			FEScene* NewScene = SCENE_MANAGER.CreateScene("Test scene");
			TestSceneID = NewScene->GetObjectID();
			ClearAllCallbacks();
			ClearNodeWidgets();

			InitiateTestScene();
		}
		else
		{
			ClearAllCallbacks();
			ClearNodeWidgets();
			DebugNodeWidgets.clear();

			FEScene* TestScene = GetTestScene();
			if (TestScene != nullptr)
				SCENE_MANAGER.DeleteScene(TestScene);
		}
#endif
	}
}

#ifdef SCENE_GRAPH_UI_WITH_FOCAL_ENGINE
bool FESceneGraphUI::InitiateTestScene()
{
	FEScene* SceneToWorkWith = GetTestScene();
	if (SceneToWorkWith == nullptr)
		return false;

	std::vector<FENaiveSceneGraphNode*> Nodes;
	for (size_t i = 0; i < 30; i++)
	{
		FEEntity* Entity = SceneToWorkWith->CreateEntity("Node_" + std::to_string(i));
		Nodes.push_back(SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity->GetObjectID()));
	}

	// Create a hierarchy:
	//
	//                  0
	//          /       |       \
    //         1        2        3
	//       / | \    / | \    / | \
    //      4  5  6  7  8  9  10 11 12
	//     /\  |     |  |  |   |  |  |\
    //   13 14 15   16 17 18  19 20 21 22
	//   |     |     |     |      |  |  \
    //  23    24    25    26      27 28  29

	// Level 1
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[1]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[2]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[3]->GetObjectID(), Nodes[0]->GetObjectID());

	// Level 2
	for (int i = 1; i <= 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			SceneToWorkWith->SceneGraph.MoveNode(Nodes[3 * i + j + 1]->GetObjectID(), Nodes[i]->GetObjectID());
		}
	}

	// Level 3
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[13]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[14]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[15]->GetObjectID(), Nodes[5]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[16]->GetObjectID(), Nodes[7]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[17]->GetObjectID(), Nodes[8]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[18]->GetObjectID(), Nodes[9]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[19]->GetObjectID(), Nodes[10]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[20]->GetObjectID(), Nodes[11]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[21]->GetObjectID(), Nodes[12]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[22]->GetObjectID(), Nodes[12]->GetObjectID());

	// Level 4
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[23]->GetObjectID(), Nodes[13]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[24]->GetObjectID(), Nodes[15]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[25]->GetObjectID(), Nodes[16]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[26]->GetObjectID(), Nodes[18]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[27]->GetObjectID(), Nodes[20]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[28]->GetObjectID(), Nodes[21]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[29]->GetObjectID(), Nodes[22]->GetObjectID());

	return true;
}

FEScene* FESceneGraphUI::GetTestScene()
{
	return SCENE_MANAGER.GetSceneByID(TestSceneID);
}

FETexture* FESceneGraphUI::GetDebugIconByIndex(const size_t& IconIndex)
{
	FETexture* Result = RESOURCE_MANAGER.NoTexture;
	if (IconIndex >= DebugIconsIDs.size())
		return Result;

	FETexture* FoundTexture = RESOURCE_MANAGER.GetTexture(DebugIconsIDs[IconIndex]);
	if (FoundTexture != nullptr)
		Result = FoundTexture;

	return Result;
}

FETexture* FESceneGraphUI::GetRandomDebugIcon()
{
	size_t Seed = static_cast<size_t>(time(nullptr));
	srand(static_cast<unsigned int>(Seed));
	if (DebugIconsIDs.size() == 0)
		return RESOURCE_MANAGER.NoTexture;

	size_t RandomIndex = rand() % DebugIconsIDs.size();
	return GetDebugIconByIndex(RandomIndex);
}

std::vector<std::string> FESceneGraphUI::GetDebugIconsIDs() const
{
	return DebugIconsIDs;
}

void FESceneGraphUI::SetDebugIconsIDs(const std::vector<std::string>& NewDebugIconsIDs)
{
	DebugIconsIDs = NewDebugIconsIDs;
}
#endif