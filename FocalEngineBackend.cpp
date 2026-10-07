#pragma once
#include "FocalEngineBackend.h"

FESceneGraphBackend::FESceneGraphBackend() {}

bool FESceneGraphBackend::IsReady() const
{
    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return false;

    return Graph != nullptr;
}

void FESceneGraphBackend::SetSceneID(const FEUUID& NewSceneID)
{
    SceneID = NewSceneID;
    Graph = &SCENE_MANAGER.GetSceneByID(SceneID)->SceneGraph;
}

SceneGraphUI::NodeHandle FESceneGraphBackend::GetRoot()
{
    return { Graph->GetRoot(), this };
}

std::vector<SceneGraphUI::NodeHandle> FESceneGraphBackend::GetChildren(SceneGraphUI::NodeHandle Node)
{
    std::vector<SceneGraphUI::NodeHandle> Result;
    for (FENaiveSceneGraphNode* Child : Node.As<FENaiveSceneGraphNode>()->GetChildren())
        Result.push_back({ Child, this });

    return Result;
}

SceneGraphUI::NodeHandle FESceneGraphBackend::GetParent(SceneGraphUI::NodeHandle Node)
{
    return { Node.As<FENaiveSceneGraphNode>()->GetParent(), this };
}

SceneGraphUI::NodeHandle FESceneGraphBackend::GetNodeByID(const FEUUID& ID)
{
    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return { nullptr, this };

    FENaiveSceneGraphNode* CurrentNode = Scene->SceneGraph.GetNodeByID(ID);
    if (CurrentNode == nullptr)
    {
        // It might be that the node is alive, but node's scene is not in focus.
        Scene = SCENE_MANAGER.GetSceneByNodeID(ID);
        if (Scene == nullptr)
            return { nullptr, this };

        CurrentNode = Scene->SceneGraph.GetNodeByID(ID);
        if (CurrentNode == nullptr)
            return { nullptr, this };
    }

    return { CurrentNode, this };
}

FEUUID FESceneGraphBackend::GetNodeID(SceneGraphUI::NodeHandle Node)
{
    return Node.As<FENaiveSceneGraphNode>()->GetID();
}

std::string FESceneGraphBackend::GetNodeName(SceneGraphUI::NodeHandle Node)
{
    FEEntity* Entity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
    if (Entity == nullptr)
    {
        FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
        if (Scene == nullptr)
            return "";

        return Scene->GetName();
    }

    return Entity->GetName();
}

std::string FESceneGraphBackend::GetTag(SceneGraphUI::NodeHandle Node)
{
    FEEntity* Entity = Node.As<FENaiveSceneGraphNode>()->GetEntity();
    if (Entity == nullptr)
        return "";

    return Entity->GetTag();
}

bool FESceneGraphBackend::MoveNode(SceneGraphUI::NodeHandle Node, SceneGraphUI::NodeHandle NewParent)
{
    return Graph->MoveNode(Node.GetID(), NewParent.GetID());
}

bool FESceneGraphBackend::IsAlive(SceneGraphUI::NodeHandle Node)
{
    if (!Node)
        return false;

    FEScene* Scene = SCENE_MANAGER.GetSceneByID(SceneID);
    if (Scene == nullptr)
        return false;

    FENaiveSceneGraphNode* CurrentNode = Scene->SceneGraph.GetNodeByID(Node.GetID());
    if (CurrentNode == nullptr)
    {
		// It might be that the node is alive, but node's scene is not in focus.
        Scene = SCENE_MANAGER.GetSceneByNodeID(Node.GetID());
		if (Scene == nullptr)
			return false;

		CurrentNode = Scene->SceneGraph.GetNodeByID(Node.GetID());
		if (CurrentNode == nullptr)
			return false;
    }

    return true;
}