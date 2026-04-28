#pragma once
#include <string>
#include <vector>
#include <functional>
#include <random>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

namespace SceneGraphUI
{
    class BackendInterface;
    class NodeHandle
    {
        void* Node = nullptr;
        std::string NodeID;
        BackendInterface* Backend = nullptr;
    public:
        NodeHandle() = default;
        NodeHandle(void* InNode, BackendInterface* InBackend);

        bool WasInitialized() const;
        explicit operator bool() const { return WasInitialized(); }

        void* Raw() const;
        BackendInterface* GetBackend() const;

        template <typename T>
        T* As() const
        {
            return static_cast<T*>(Node);
        }

        std::string GetID() const;
        std::string GetName() const;
        NodeHandle GetParent() const;
        std::vector<NodeHandle> GetChildren() const;
        size_t GetDepth() const;
        std::string GetTag() const;

        bool operator==(const NodeHandle& Other) const
        {
            return Node == Other.Node && NodeID == Other.NodeID;
        }

		bool operator!=(const NodeHandle& Other) const
        {
            return !(*this == Other);
        }
    };

    class BackendInterface
    {
		friend class NodeHandle;
        friend struct NodeWidget;
		friend class TreeView;

        static std::string GetUniqueID();
        // This function can produce ID's that are "unique" with very rare collisions.
        // For most purposes it can be considered unique.
        // ID is a 24 long string.
        static std::string GetUniqueHexID();

        enum class EllipsisPosition
        {
            End,
            Middle,
            Start
        };

        static std::string TruncateText(const std::string& Text, float MaxWidth, EllipsisPosition Position = EllipsisPosition::End, const std::string& Ellipsis = "...");
    public:
        virtual ~BackendInterface() = default;

        virtual bool IsReady() const = 0;

        virtual NodeHandle GetRoot() = 0;
        virtual std::vector<NodeHandle> GetChildren(NodeHandle Node) = 0;
        virtual NodeHandle GetParent(NodeHandle Node) = 0;

        virtual NodeHandle GetNodeByID(const std::string& ID) = 0;
        virtual std::string GetNodeID(NodeHandle Node) = 0;
        virtual std::string GetNodeName(NodeHandle Node) = 0;

        virtual size_t GetDepth(NodeHandle Node);

        virtual std::string GetTag(NodeHandle);
        virtual bool Rename(NodeHandle, const std::string&);
        virtual bool MoveNode(NodeHandle Node, NodeHandle NewParent);

        virtual bool IsAlive(NodeHandle Node) = 0;
    };
}