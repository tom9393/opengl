#ifndef PATH
#define PATH
#include "transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <vector>

class Path {
public:
    // Constructor from a list of control points
    Path(const std::vector<glm::vec3>& points) : controlPoints(points), currentTargetIndex(0), looping(false) {}

    // Set or replace the entire list of control points
    void setControlPoints(const std::vector<glm::vec3>& points) {
        controlPoints = points;
        currentTargetIndex = 0;
    }

    // Returns the current target control point
    const glm::vec3& getCurrentTarget() const {
        if (controlPoints.empty()) {
            static glm::vec3 dummy(0.0f);
            return dummy;
        }
        return controlPoints[currentTargetIndex];
    }

    // Advance to the next control point if within threshold distance
    void advanceIfClose(const glm::vec3& pos, float threshold) {
        if (controlPoints.empty()) return;

        float dist2 = glm::distance2(pos, getCurrentTarget());
        if (dist2 < threshold * threshold) {
            if (currentTargetIndex + 1 < controlPoints.size()) {
                currentTargetIndex++;
            } else if (looping) {
                currentTargetIndex = 0;
            }
            // Else: stay at last point
        }
    }

    // Restart from the first control point
    void reset() {
        currentTargetIndex = 0;
    }

    // Enable or disable looping
    void loop(bool enable) {
        looping = enable;
    }

    // Returns true if the path has reached the end and is not looping
    bool isAtEnd() const {
        return currentTargetIndex >= controlPoints.size() - 1 && !looping;
    }

    // Returns the full path (read-only)
    const std::vector<glm::vec3>& getControlPoints() const {
        return controlPoints;
    }

    // Get index of current target
    size_t getCurrentTargetIndex() const {
        return currentTargetIndex;
    }

private:
    std::vector<glm::vec3> controlPoints;
    size_t currentTargetIndex = 0;
    bool looping = false;
};



#endif
