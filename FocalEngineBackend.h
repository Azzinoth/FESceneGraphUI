#pragma once
#include "BackendInterface.h"
#include "FEngine.h"

class FESceneGraphBackend : public SceneGraphUI::BackendInterface
{
	FEUUID SceneID;
    FENaiveSceneGraph* Graph = nullptr;
public:
    FESceneGraphBackend();

    bool IsReady() const override;
    bool IsAlive(SceneGraphUI::NodeHandle Node) override;

    void SetSceneID(const FEUUID& NewSceneID);
    SceneGraphUI::NodeHandle GetRoot() override;

    std::vector<SceneGraphUI::NodeHandle> GetChildren(SceneGraphUI::NodeHandle Node) override;
    SceneGraphUI::NodeHandle GetParent(SceneGraphUI::NodeHandle Node) override;

    SceneGraphUI::NodeHandle GetNodeByID(const FEUUID& ID) override;
    FEUUID GetNodeID(SceneGraphUI::NodeHandle Node) override;

    std::string GetNodeName(SceneGraphUI::NodeHandle Node) override;
    std::string GetTag(SceneGraphUI::NodeHandle Node) override;

    bool MoveNode(SceneGraphUI::NodeHandle Node, SceneGraphUI::NodeHandle NewParent) override;
};