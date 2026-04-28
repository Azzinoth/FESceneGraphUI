#pragma once
#include "BackendInterface.h"
#ifdef SCENE_GRAPH_UI_WITH_FOCAL_ENGINE
#include "FEngine.h"
#endif

namespace SceneGraphUI
{
	struct NodeStateData
	{
		bool bExpanded = false;
		bool bSelected = false;
	};

	struct NodeWidget
	{
		friend class TreeView;
	private:
		std::string ID;

	public:
		NodeWidget::NodeWidget()
		{
			ID = SceneGraphUI::BackendInterface::GetUniqueHexID();
		}

		std::string GetID() const
		{
			return ID;
		}

		std::string TooltipText = "";

		ImTextureID Icon = 0;
		std::function<ImTextureID (SceneGraphUI::NodeHandle)> DynamicIconProvider = nullptr;

		ImVec4 HoveredColor = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		ImVec4 ActiveColor = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);

		bool bIsInteractive = false;
		std::function<void(SceneGraphUI::NodeHandle)> OnClickCallback = nullptr;
		bool bIsVisibleByDefault = true;
		std::function<bool(SceneGraphUI::NodeHandle)> IsVisiblePredicate = nullptr;
	};

	class TreeView
	{
		// Core.
		SceneGraphUI::BackendInterface* Backend = nullptr;
		bool bVisible = true;
		SceneGraphUI::NodeHandle RenderingRoot;
		bool bShowRoot = false;
		void RenderNode(SceneGraphUI::NodeHandle Node);


		// Appearance.
		bool bBackgroundColorSwitch = true;
		ImVec4 BackgroundColor = ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
		ImVec4 EvenNodeBackgroundColor = ImVec4(50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f);
		ImVec4 OddNodeBackgroundColor = ImVec4(90.0f / 255.0f, 90.0f / 255.0f, 90.0f / 255.0f, 1.0f);
		ImVec4 ConnectorLineColor = ImVec4(128.0f / 255.0f, 140.0f / 255.0f, 170.0f / 255.0f, 1.0f);
		float ConnectorLineThickness = 2.0f;
		ImVec4 SelectedNodeConnectorLineColor = ImVec4(48.0f / 255.0f, 95.0f / 255.0f, 213.0f / 255.0f, 1.0f);
		float SelectedConnectorLineThickness = 2.6f;
		bool bAlternatingNodeBackground = true;
		//bool bOnlyTextPartOfNodeUsesBackground = true;
		static bool bLastNodeWasEven;

		ImFont* DefaultFont = nullptr;
		float FontSize = 32.0f;
		float TooltipFontSize = FontSize / 2.0f;
		float NodeHeight = 32.0f;
		ImVec2 IconsSize = ImVec2(NodeHeight, NodeHeight);
		bool bRenderUIScaleControl = true;
		void RenderUIScaleControl(float Min = 8.0f, float Max = 128.0f);


		// Node state (expand/collapse/selection).
		std::unordered_map<std::string, NodeStateData> NodeState;
		bool bAllowMultipleNodeSelection = false;
		std::function<bool(SceneGraphUI::NodeHandle)> NodeSelectionPredicate = nullptr;
		std::vector<std::function<void(SceneGraphUI::NodeHandle, bool)>> OnNodeSelectionChangedCallbacks;
		void SetNodeSelectedInternal(SceneGraphUI::NodeHandle Node, bool bSelected);


		// Predicates and providers.
		std::function<bool(SceneGraphUI::NodeHandle)> NodeRenderPredicate = nullptr;
		std::function<std::string(SceneGraphUI::NodeHandle)> NodeDisplayNameProvider = nullptr;
		std::string GetNodeDisplayName(SceneGraphUI::NodeHandle Node);
		std::function<bool(SceneGraphUI::NodeHandle)> NodeChildrenVisiblePredicate = nullptr;
		std::function<ImTextureID (SceneGraphUI::NodeHandle)> NodeIconProvider = nullptr;
		ImTextureID GetNodeIcon(SceneGraphUI::NodeHandle Node);


		// Visibility/filtering.
		std::vector<std::string> HiddenEntityTags;
		bool ShouldNodeBeVisible(SceneGraphUI::NodeHandle Node);
		bool AreNodeChildrenVisible(SceneGraphUI::NodeHandle Node);

		bool bRenderTextFilterInput = true;
		bool bFilterEnabled = false;
		bool bCaseSensitiveFiltering = false;
		std::string FilterText = "";
		bool DoesNodePassTextFilter(SceneGraphUI::NodeHandle Node);
		void RenderFilterTextInput();
		static constexpr size_t FilterInputBufferSize = 2048;
		char FilterBuffer[FilterInputBufferSize];
		bool bIsPlaceHolderTextUsed = true;
		bool bFilterInputWasFocused = false;
		std::string PlaceholderText = "Filter entities...";
		int FilterInputTextCallback(ImGuiInputTextCallbackData* Data);


		// Tree visualization.
		float TreeArrowsThicknessCoefficient = 0.07f;
		float LineJoinOverlapFactor = 0.77f;
		bool IsNodePartOfBranch(SceneGraphUI::NodeHandle NodeToCheck, SceneGraphUI::NodeHandle BranchRoot, SceneGraphUI::NodeHandle BranchLeaf);
		bool bHighlightSelectedNodeConnectorLines = true;
		void DrawTreeConnectorLines(SceneGraphUI::NodeHandle Node, float ParentBottomY);
		void DrawAppropriateTreeArrow(SceneGraphUI::NodeHandle Node);


		// Input handling.
		bool bWindowHovered = false;
		std::string HoveredNodeID = "";
		std::string HoveredNodeIDWhenContextMenuWasOpened = "";
		bool bShouldOpenContextMenu = false;
		std::function<void(SceneGraphUI::NodeHandle)> ContextMenuRenderer = nullptr;
		void RenderContextMenu();

		std::vector<std::function<void(SceneGraphUI::NodeHandle)>> OnNodeHoveredCallbacks;
		std::vector<std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)>> OnNodeClickedCallbacks;
		std::vector<std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)>> OnNodeDoubleClickedCallbacks;
		void CheckInputs(SceneGraphUI::NodeHandle Node);


		// Before/After render callbacks.
		std::vector<std::function<void(SceneGraphUI::NodeHandle)>> BeforeNodeRenderCallbacks;
		std::vector<std::function<void(SceneGraphUI::NodeHandle)>> AfterNodeRenderCallbacks;


		// Renaming functionality.
		std::string NodeIDBeingRenamed = "";
		char RenameBuffer[1024];
		bool bLastFrameRenameEditWasVisible = false;
		std::function<void(SceneGraphUI::NodeHandle, std::string)> RenameNodeFunction = nullptr;


		// Node widgets.
		std::vector<NodeWidget> NodeWidgets;
		float WidgetIconScale = 0.9f;
		float YCursorPositionBeforeRenderingWidgets = 0.0f;
		float YCursorPositionAfterRenderingWidgets = 0.0f;

		bool ShouldRenderWidgetForNode(SceneGraphUI::NodeHandle Node, NodeWidget& Widget, ImTextureID& IconToUse);
		float GetNodeWidgetAreaWidth(SceneGraphUI::NodeHandle Node);
		size_t GetNodeWidgetCount(SceneGraphUI::NodeHandle Node);
		void RenderNodeWidgets(SceneGraphUI::NodeHandle Node);


		// Debug stuff.
		bool bDebugMode = false;
		bool bDebugRenderRoot = true;
		bool bDebugRenderRandomNodeIcons = false;
#ifdef SCENE_GRAPH_UI_WITH_FOCAL_ENGINE
		std::vector<std::string> DebugIconsIDs;
		FETexture* GetDebugIconByIndex(const size_t& IconIndex);
		FETexture* GetRandomDebugIcon();
		void DebugCreateRandomWidgets(bool bInteractive);

		std::vector<NodeWidget> DebugNodeWidgets;

		std::string TestSceneID;
		FEScene* GetTestScene();
		bool InitiateTestScene();
		void DebugRenderUI();
#endif
	public:
		TreeView(SceneGraphUI::BackendInterface* Backend);
		~TreeView();

		static std::string GetFullVersion();
		SceneGraphUI::NodeHandle GetRenderingRoot() const;

		void Render(SceneGraphUI::NodeHandle RenderingRoot, bool bRenderRootItself = true);

		float GetFontSize() const;
		void SetFontSize(float NewFontSize);

		void SetNodeRenderPredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate);
		void SetNodeDisplayNameProvider(std::function<std::string(SceneGraphUI::NodeHandle)> Provider);
		void SetNodeChildrenVisiblePredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate);
		void SetNodeSelectionPredicate(std::function<bool(SceneGraphUI::NodeHandle)> Predicate);
		void SetNodeIconProvider(std::function<ImTextureID(SceneGraphUI::NodeHandle)> Provider);
		void ClearAllProvidersAndPredicates();

		void AddBeforeNodeRenderCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback);
		void AddAfterNodeRenderCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback);

		std::vector<std::string> GetSelectedNodeIDs() const;
		bool IsNodeSelected(SceneGraphUI::NodeHandle Node);
		void SetNodeSelected(SceneGraphUI::NodeHandle Node, bool bSelected);
		bool IsNodeExpanded(SceneGraphUI::NodeHandle Node);
		void SetNodeExpanded(SceneGraphUI::NodeHandle Node, bool bExpanded);
		bool IsNodeExpandedTo(SceneGraphUI::NodeHandle Node);
		void ExpandToNode(SceneGraphUI::NodeHandle Node);
		void ExpandAllNodes();
		void CollapseAllNodes();

		std::vector<std::string> GetHiddenEntityTags() const;
		void SetHiddenEntityTags(const std::vector<std::string>& NewHiddenEntityTags);
		void AddHiddenEntityTag(const std::string& TagToAdd);
		void RemoveHiddenEntityTag(const std::string& TagToRemove);
		void ClearHiddenEntityTags();

		NodeWidget* GetNodeWidgetByID(const std::string& WidgetID);
		std::vector<NodeWidget> GetAllNodeWidgets() const;
		bool AddNodeWidget(NodeWidget& Widget);
		bool RemoveNodeWidget(const std::string& WidgetID);
		void ClearNodeWidgets();

		void AddOnNodeHoveredCallback(std::function<void(SceneGraphUI::NodeHandle)> Callback);
		void ClearOnNodeHoveredCallbacks();

		void AddOnNodeClickedCallback(std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)> Callback);
		void ClearOnNodeClickedCallbacks();

		void AddOnNodeDoubleClickedCallback(std::function<void(SceneGraphUI::NodeHandle, ImGuiMouseButton_)> Callback);
		void ClearOnNodeDoubleClickedCallbacks();

		void AddOnNodeSelectionChangedCallback(std::function<void(SceneGraphUI::NodeHandle, bool)> Callback);
		void ClearOnNodeSelectionChangedCallbacks();

		void ClearAllInputCallbacks();
		void ClearAllCallbacks();

		void SetContextMenuRenderingFunction(std::function<void(SceneGraphUI::NodeHandle)> Function);
		void ClearContextMenuRenderingFunction();

		bool IsNodeBeingRenamed(SceneGraphUI::NodeHandle Node);
		bool SetNodeForRenaming(SceneGraphUI::NodeHandle Node);
		void SetRenameNodeFunction(std::function<void(SceneGraphUI::NodeHandle, std::string)> Function);
		void ClearRenameNodeFunction();

		bool IsInDebugMode();
		void SetDebugMode(bool bNewValue);
#ifdef SCENE_GRAPH_UI_WITH_FOCAL_ENGINE
		std::vector<std::string> GetDebugIconsIDs() const;
		void SetDebugIconsIDs(const std::vector<std::string>& NewDebugIconsIDs);
#endif
	};
}