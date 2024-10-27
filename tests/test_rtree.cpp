#include <doctest/doctest.h>

#include "custom_allocator.h"

#define SPATIAL_TREE_ALLOCATOR 2

#include <THST/RTree.h>
#include <iostream>
#include <numeric>
#include <sstream>

template <typename T> struct Point {
    union {
        T data[2];
        struct {
            T x, y;
        };
    };

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    inline float distance(const Point point) const {

        float d = float(data[0] - point.data[0]);
        d *= d;
        for (int i = 1; i < 2; i++)
        {
            float temp = float(data[i] - point.data[i]);
            d += temp * temp;
        }
        return std::sqrt(d);
    }
};

template <typename T> struct Box2 {
    T min[2];
    T max[2];

    explicit operator spatial::BoundingBox<int, 2>() const {
        return spatial::BoundingBox<int, 2>(min, max);
    }

    bool operator==(const Box2& other) const
    {
        return min[0] == other.min[0] && min[1] == other.min[1] && max[0] == other.max[0] && max[1] == other.max[1];
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& stream, const Box2<T>& bbox) {
    stream << "min: " << bbox.min[0] << " " << bbox.min[1] << " ";
    stream << "max: " << bbox.max[0] << " " << bbox.max[1];
    return stream;
}

struct Object {
    spatial::BoundingBox<int, 2> bbox;
    std::string name;

    // needed to avoid adding duplicates
    bool operator==(const Object& other) const
    {
        return name == other.name;
    }
};

typedef spatial::RTree<int, Box2<int>, 2, 4, 2> rtree_box_t;

const Box2<int> kBoxes[] = {
    {{5, 2}, {16, 7}},  {{1, 1}, {2, 2}},  {{26, 24}, {44, 28}}, {{22, 21}, {23, 24}},
    {{16, 0}, {32, 16}},   {{0, 0}, {8, 8}},      {{4, 4}, {6, 8}}, {{2, 1}, {2, 3}},
    {{4, 2}, {8, 4}},      {{3, 3}, {12, 16}},    {{0, 0}, {64, 32}}, {{3, 2}, {32, 35}},
    {{32, 32}, {64, 128}}, {{128, 0}, {256, 64}}, {{120, 64}, {250, 128}}, {{123, 84}, {230, 122}} };

// use a custom indexable for custom objects
struct Indexable {
    const int* min(const Object& value) const { return value.bbox.min; }
    const int* max(const Object& value) const { return value.bbox.max; }
};

std::vector<Object> objects;
std::vector<Box2<int>> boxes;
rtree_box_t rtree;

TEST_CASE("test rtree create") {

    size_t i = 0;
    MESSAGE("Creating objects:\n");
    for (const auto& bbox : kBoxes) {
        boxes.push_back(bbox);
        objects.emplace_back();
        Object& obj = objects.back();

        std::stringstream ss;
        ss << "object" << i++;
        obj.name = ss.str();
        obj.bbox = ( spatial::BoundingBox<int, 2> )bbox;

        MESSAGE(obj.name << " " << obj.bbox << "\n");
    }

    // create a quad tree with the given box
    spatial::BoundingBox<int, 2> bbox(( spatial::box::empty_init() ));
    Point<int> point;
    point.set(0, 0);
    bbox.extend(point.data);
    point.set(256, 128);
    bbox.extend(point.data);

    CHECK(rtree.count() == 0u);
    rtree = rtree_box_t(std::begin(kBoxes), std::end(kBoxes));
    CHECK(rtree.count() == 16u);
    rtree.clear();
    CHECK(rtree.count() == 0u);

    // or construction via insert
    rtree.insert(std::begin(kBoxes), std::end(kBoxes));
    CHECK(rtree.count() == 16u);
    Box2<int> box = { {7, 3}, {14, 6} };
    rtree.insert(box);
    CHECK(rtree.count() == 17u);

    SUBCASE("conditional insert")
    {
        // insert only if predicate is always valid
        Box2<int> box2 = { {7, 4}, {14, 6} };
        bool wasAdded =
            rtree.insert(box2, [&box2](const decltype( rtree )::bbox_type& bbox) {
            const decltype( rtree )::bbox_type cbbox(box2.min, box2.max);
            return !bbox.overlaps(cbbox);
                });
        CHECK(!wasAdded);
        CHECK(rtree.count() == 17u);

        wasAdded =
            rtree.insert(box, [&box](const decltype( rtree )::bbox_type& bbox) {
            const decltype( rtree )::bbox_type cbbox(box.min, box.max);
            return !bbox.overlaps(cbbox);
                });
        CHECK(!wasAdded);
        CHECK(rtree.count() == 17u);
    }
}

TEST_CASE("test rtree create") {
    Box2<int> box = { {7, 3}, {14, 6} };

    MESSAGE("Created trees, element count: " << rtree.count() << "\n");
    CHECK(rtree.count() == 17u);
    rtree_box_t::bbox_type treeBBox = rtree.bbox();
    CHECK(treeBBox.min[0] == 0);
    CHECK(treeBBox.min[1] == 0);
    CHECK(treeBBox.max[0] == 256);
    CHECK(treeBBox.max[1] == 128);

    // remove an element
    bool removed = rtree.remove(box);
    CHECK(removed);
    CHECK(rtree.count() == 16u);

    std::vector<Box2<int>> results;
    removed = rtree.remove(box);
    CHECK(!removed);
    results.clear();
    rtree.query(spatial::contains<2>(box.min, box.max), std::back_inserter(results));
    CHECK(results.empty());

    box = { {0, 0}, {20, 50} };
    rtree.query(spatial::contains<2>(box.min, box.max), std::back_inserter(results));
    CHECK(!results.empty());
    CHECK(results.size() == 7);
}

TEST_CASE("query for results within the search box")
{
    Box2<int> searchBox = { {0, 0}, {8, 31} };
    std::vector<Box2<int>> results;
    rtree.query(spatial::intersects<2>(searchBox.min, searchBox.max),
        std::back_inserter(results));

    std::stringstream resultsStream;
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 3 3 max: 12 16, min: 0 0 max: 64 32, min: 3 2 max: 32 35, min: 1 1 max: 2 2, min: 2 1 max: 2 3, min: 5 2 max: 16 7, min: 0 0 max: 8 8, min: 4 4 max: 6 8, min: 4 2 max: 8 4, ");

    results.clear();
    rtree.query(spatial::contains<2>(searchBox.min, searchBox.max),
        std::back_inserter(results));
    resultsStream.clear();

    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 3 3 max: 12 16, min: 0 0 max: 64 32, min: 3 2 max: 32 35, min: 1 1 max: 2 2, min: 2 1 max: 2 3, min: 5 2 max: 16 7, min: 0 0 max: 8 8, min: 4 4 max: 6 8, min: 4 2 max: 8 4, min: 1 1 max: 2 2, min: 2 1 max: 2 3, min: 0 0 max: 8 8, min: 4 4 max: 6 8, min: 4 2 max: 8 4, ");
}

TEST_CASE("ray query")
{
    Point<float> rayOrigin({ 0.5, 0 });
    Point<float> rayDir({ 0, 1 });
    std::vector<Box2<int>> results;
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));

    std::stringstream resultsStream;
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 0 0 max: 64 32, min: 0 0 max: 8 8, ");

    rayOrigin = Point<float>({ 100.5, 0.5 });
    rayDir = Point<float>({ 0, 1 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    CHECK(results.empty());

    rayOrigin = Point<float>({ 68,130 });
    rayDir = Point<float>({ 120, 52 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    resultsStream.str("");
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 32 32 max: 64 128, ");

    rayOrigin = Point<float>({ 11,110 });
    rayDir = Point<float>({ 1, 0 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    resultsStream.str("");
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 32 32 max: 64 128, min: 123 84 max: 230 122, min: 120 64 max: 250 128, ");

    rayOrigin = Point<float>({ 63, 25 });
    rayDir = Point<float>({ 0, 2 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    resultsStream.str("");
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 32 32 max: 64 128, min: 0 0 max: 64 32, ");

    rayOrigin = Point<float>({ 62, 70 });
    rayDir = Point<float>({ 0, -2 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    resultsStream.str("");
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 32 32 max: 64 128, min: 0 0 max: 64 32, ");

    auto fnTestPredicate = [](const Box2<int>& box) {return box.min[0] == 0;  };
    rayOrigin = Point<float>({ 62, 70 });
    rayDir = Point<float>({ 0, -2 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results), fnTestPredicate);
    resultsStream.str("");
    for (const auto& res : results)
        resultsStream << res << ", ";
    CHECK(resultsStream.str() == "min: 0 0 max: 64 32, ");

    rayOrigin = Point<float>({ 65, 70 });
    rayDir = Point<float>({ 0, -2 });
    results.clear();
    rtree.rayQuery(rayOrigin.data, rayDir.data, std::back_inserter(results));
    CHECK(results.empty());
}

TEST_CASE("tree traversal")
{
    spatial::RTree<int, Object, 2, 4, 2, Indexable> rtree;
    rtree.insert(objects.begin(), objects.end());

    SUBCASE("leaf traversal of the tree")
    {
        spatial::RTree<int, Object, 2, 4, 2, Indexable> rtree;
        rtree.insert(objects.begin(), objects.end());

        std::stringstream resultsStream;
        for (const auto& obj : objects) {
            resultsStream << obj.name << " ";
        }
        CHECK(resultsStream.str() == "object0 object1 object2 object3 object4 object5 object6 object7 object8 object9 object10 object11 object12 object13 object14 object15 ");

        // gives the spatial partitioning order within the tree
        resultsStream.clear();
        for (auto it = rtree.lbegin(); it.valid(); it.next()) {
            resultsStream << ( *it ).name << " ";
        }
        CHECK(resultsStream.str() == "object0 object1 object2 object3 object4 object5 object6 object7 object8 object9 object10 object11 object12 object13 object14 object15 object2 object3 object12 object15 object9 object10 object11 object13 object14 object1 object7 object0 object4 object5 object6 object8 ");
    }

    SUBCASE("depth traversal of the tree")
    {
        CHECK(rtree.levels() > 0);
        std::stringstream resultsStream;
        for (auto it = rtree.dbegin(); it.valid(); it.next()) {
            std::string parentName;
            spatial::BoundingBox<int, 2> parentBBox;

            // traverse current children of the parent node(i.e. upper level)
            for (auto nodeIt = it.child(); nodeIt.valid(); nodeIt.next()) {
                resultsStream << "level: " << nodeIt.level() << " " << ( *nodeIt ).name << " | ";
                parentName += ( *nodeIt ).name + " + ";
                parentBBox.extend(nodeIt.bbox());
            }
            ( *it ).name = parentName;
            ( *it ).bbox = parentBBox;
            resultsStream << "level: " << it.level() << " " << parentName << " \n ";
        }
        CHECK(resultsStream.str() == "level: 0 object2 | level: 0 object3 | level: 0 object12 | level: 0 object15 | level: 1 object2 + object3 + object12 + object15 +  \n level: 0 object9 | level: 0 object10 | level: 0 object11 | level: 1 object9 + object10 + object11 +  \n level: 0 object13 | level: 0 object14 | level: 1 object13 + object14 +  \n level: 1 object2 + object3 + object12 + object15 +  | level: 1 object9 + object10 + object11 +  | level: 1 object13 + object14 +  | level: 2 object2 + object3 + object12 + object15 +  + object9 + object10 + object11 +  + object13 + object14 +  +  \n level: 0 object1 | level: 0 object7 | level: 1 object1 + object7 +  \n level: 0 object0 | level: 0 object4 | level: 1 object0 + object4 +  \n level: 0 object5 | level: 0 object6 | level: 0 object8 | level: 1 object5 + object6 + object8 +  \n level: 1 object1 + object7 +  | level: 1 object0 + object4 +  | level: 1 object5 + object6 + object8 +  | level: 2 object1 + object7 +  + object0 + object4 +  + object5 + object6 + object8 +  +  \n ");
    }

    SUBCASE("special hierarchical query")
    {
        std::vector<Object> results;
        Box2<int> searchBox = { {4, 14}, {8, 31} };
        rtree.hierachical_query(
            spatial::intersects<2>(searchBox.min, searchBox.max),
            std::back_inserter(results));
        std::stringstream resultsStream;
        for (const auto& res : results)
            resultsStream << res.name << " | ";

        CHECK(resultsStream.str() == "object9 | object10 | object11 | ");
    }
}

TEST_CASE("nearest neighbor query")
{
    spatial::RTree<int, Object, 2, 4, 2, Indexable> rtree;
    rtree.insert(objects.begin(), objects.end());
    Point<int> p = { {0, 0} };
    SUBCASE("nearest neighbor radius query")
    {
        std::vector<Object> results;
        rtree.nearest(p.data, 100, std::back_inserter(results));
        std::stringstream resultsStream;
        for (const auto& res : results) {
            Point<int>  center;
            res.bbox.center(center.data);
            resultsStream << res.name << " : " << p.distance(center) << " | ";
        }
        CHECK(resultsStream.str() == "object1 : 1.41421 | object7 : 2.82843 | object5 : 5.65685 | object8 : 6.7082 | object6 : 7.81025 | object0 : 10.7703 | object4 : 25.2982 | object9 : 11.4018 | object11 : 24.7588 | object10 : 35.7771 | object3 : 31.1127 | object2 : 43.6005 | object12 : 93.2952 | ");
    }

    SUBCASE("Nearest knn query:")
    {
        std::stringstream resultsStream;
        std::vector<Object> results;
        rtree.k_nearest(p.data, 3, std::back_inserter(results));
        for (const auto& res : results)
            resultsStream << res.name << " : " << res.bbox.distance(p.data) << " | ";
        CHECK(resultsStream.str() == "object10 : 0 | object5 : 0 | object1 : 1.41421 | ");
    }
}

TEST_CASE("custom indexable for array and indices as values")
{
    struct ArrayIndexable {

        ArrayIndexable(const std::vector<Box2<int>>& array) : array(array) {}

        const int* min(const uint32_t index) const { return array[index].min; }
        const int* max(const uint32_t index) const { return array[index].max; }

        const std::vector<Box2<int>>& array;
    };

    ArrayIndexable indexable(boxes);

    typedef uint32_t IndexType;
    spatial::RTree<int, IndexType, 2, 4, 2, ArrayIndexable> rtree(indexable);

    std::vector<uint32_t> indices(boxes.size());
    std::iota(indices.begin(), indices.end(), 0);
    rtree.insert(indices.begin(), indices.end());

    indices.clear();
    Box2<int> searchBox = { {0, 0}, {8, 31} };
    rtree.query(spatial::intersects<2>(searchBox.min, searchBox.max),
        std::back_inserter(indices));
    std::stringstream resultsStream;
    for (auto index : indices)
        resultsStream << "index: " << index << " " << objects[index].name << " " << objects[index].bbox << " | ";
    CHECK(resultsStream.str() == "index: 9 object9 min: 3 3 max: 12 16 | index: 10 object10 min: 0 0 max: 64 32 | index: 11 object11 min: 3 2 max: 32 35 | index: 1 object1 min: 1 1 max: 2 2 | index: 7 object7 min: 2 1 max: 2 3 | index: 0 object0 min: 5 2 max: 16 7 | index: 5 object5 min: 0 0 max: 8 8 | index: 6 object6 min: 4 4 max: 6 8 | index: 8 object8 min: 4 2 max: 8 4 | ");
}

#if SPATIAL_TREE_ALLOCATOR == SPATIAL_TREE_DEFAULT_ALLOCATOR
TEST_CASE("Custom allocator test")
{
    struct ArrayIndexable {

        ArrayIndexable(const std::vector<Box2<int>>& array) : array(array) {}

        const int* min(const uint32_t index) const { return array[index].min; }
        const int* max(const uint32_t index) const { return array[index].max; }

        const std::vector<Box2<int>>& array;
    };

    const int kMaxKeysPerNode = 4;
    const int kVolumeMode = spatial::box::eSphericalVolume;

    typedef spatial::BoundingBox<int, 2> tree_bbox_type;
    typedef spatial::detail::Node<uint32_t, tree_bbox_type, kMaxKeysPerNode>
        tree_node_type;
    typedef test::tree_allocator<tree_node_type> tree_allocator_type;

    typedef spatial::RTree<int, uint32_t, 2, kMaxKeysPerNode,
        kMaxKeysPerNode / 2, ArrayIndexable, kVolumeMode,
        double, tree_allocator_type>
        CustomTree;

    ArrayIndexable indexable(boxes);
    std::vector<uint32_t> indices(boxes.size() / 2);
    CustomTree rtree(indexable, tree_allocator_type(), true);

    indices.resize(boxes.size() / 2);
    std::iota(indices.begin(), indices.end(), 0);
    rtree.insert(indices.begin(), indices.end());
    std::stringstream resultsStream;
    for (auto index : indices)
        resultsStream << "index: " << index << " " << objects[index].name << " " << objects[index].bbox << " | ";
    CHECK(resultsStream.str() == "index: 0 object0 min: 5 2 max: 16 7 | index: 1 object1 min: 1 1 max: 2 2 | index: 2 object2 min: 26 24 max: 44 28 | index: 3 object3 min: 22 21 max: 23 24 | index: 4 object4 min: 16 0 max: 32 16 | index: 5 object5 min: 0 0 max: 8 8 | index: 6 object6 min: 4 4 max: 6 8 | index: 7 object7 min: 2 1 max: 2 3 | ");
}
#endif
