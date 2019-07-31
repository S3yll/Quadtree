#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include "Quadtree.h"

using namespace ds;
using namespace math;

struct Node
{
    Box<float> box;
    std::size_t id;
};

std::vector<Node> generateRandomNodes(std::size_t n)
{
    auto generator = std::default_random_engine();
    auto originDistribution = std::uniform_real_distribution(0.0f, 1.0f);
    auto sizeDistribution = std::uniform_real_distribution(0.0f, 0.01f);
    auto nodes = std::vector<Node>(n);
    for (auto i = std::size_t(0); i < n; ++i)
    {
        nodes[i].box.left = originDistribution(generator);
        nodes[i].box.top = originDistribution(generator);
        nodes[i].box.width = std::min(1.0f - nodes[i].box.left, sizeDistribution(generator));
        nodes[i].box.height = std::min(1.0f - nodes[i].box.top, sizeDistribution(generator));
        nodes[i].id = i;
    }
    return nodes;
}

std::vector<Node*> computeIntersections(const math::Box<float>& box, std::vector<Node>& nodes, const std::vector<bool>& removed)
{
    auto intersections = std::vector<Node*>();
    for (auto& n : nodes)
    {
        if (removed.size() == 0 || !removed[n.id])
        {
            if (box.intersects(n.box))
                intersections.push_back(&n);
        }
    }
    return intersections;
}

void checkIntersections(std::vector<Node*> nodes1, std::vector<Node*> nodes2)
{
    assert(nodes1.size() == nodes2.size());
    std::sort(std::begin(nodes1), std::end(nodes1));
    std::sort(std::begin(nodes2), std::end(nodes2));
    for (auto i = std::size_t(0); i < nodes1.size(); ++i)
        assert(nodes1[i] == nodes2[i]);
}

int main()
{
    auto n = std::size_t(124);
    auto contain = [](const Box<float>& box, Node* node)
    {
        return box.contains(node->box);
    };
    auto intersect = [](const Box<float>& box, Node* node)
    {
        return box.intersects(node->box);
    };
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(n);
    // Add nodes to quadtree
    auto quadtree = Quadtree<Node*, decltype(contain), decltype(intersect)>(box, contain, intersect);
    auto start1 = std::chrono::steady_clock::now();
    for (auto& node : nodes)
        quadtree.add(&node);
    // Randomly remove some nodes
    auto generator = std::default_random_engine();
    auto deathDistribution = std::uniform_int_distribution(0, 1);
    auto removed = std::vector<bool>(nodes.size());
    std::generate(std::begin(removed), std::end(removed),
        [&generator, &deathDistribution](){ return deathDistribution(generator); });
    for (auto& node : nodes)
    {
        if (removed[node.id])
            quadtree.remove(&node);
    }
    // Quadtree
    auto intersections1 = std::vector<std::vector<Node*>>(nodes.size());
    auto start2 = std::chrono::steady_clock::now();
    for (const auto& node : nodes)
    {
        if (!removed[node.id])
            intersections1[node.id] = quadtree.query(node.box);
    }
    auto duration2 = std::chrono::steady_clock::now() - start2;
    auto duration1 = std::chrono::steady_clock::now() - start1;
    std::cout << "quadtree: " << std::chrono::duration_cast<std::chrono::microseconds>(duration2).count() << "us" << '\n';
    std::cout << "quadtree with creation: " << std::chrono::duration_cast<std::chrono::microseconds>(duration1).count() << "us" << '\n';
    // Brute force
    auto intersections2 = std::vector<std::vector<Node*>>(n);
    for (const auto& node : nodes)
    {
        if (!removed[node.id])
            intersections2[node.id] = computeIntersections(node.box, nodes, removed);
    }
    // Check
    for (const auto& node : nodes)
    {
        if (!removed[node.id])
            checkIntersections(intersections1[node.id], intersections2[node.id]);
    }

    return 0;
}