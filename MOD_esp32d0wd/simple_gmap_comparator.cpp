#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

#define GMAP_TRANS_ERR_THRESHOLD 500 // Threshold for equality

/**
 * @brief Compare two points within a threshold
 * 
 * @param bounds bounds :: int[2][2]
 * @param com gpos to comapare :: int[3]
 * @return true(1) if com within rectangular(axis aligned) bounds;
 * @return false(0) if com outside rectangular(axis aligned) bounds
 */
int simple_gmap_compare_withinBounds(int **bounds, int *com) {
    // if com is within bounds
    if (    com[0] > bounds[0][0] && com[0] < bounds[1][0] && 
            com[1] > bounds[0][1] && com[1] < bounds[1][1])
        return 1;
    else
        return 0;
}

// Structure to represent a point
struct Point {
    double x;
    double y;
    double rot; // rot value
};

// Comparator function for sorting points based on rot
bool compareTranlationX(const Point& p1, const Point& p2) {
    return (p1.x < p2.x);
}
bool compareTranlationX_NOT(const Point& p1, const Point& p2) {
    return (p1.x > p2.x);
}

// Function to calculate centroid of a set of points
Point calculateCentroid(const vector<Point>& points) {
    double centroid[3] = {0.0, 0.0, 0.0};
    for (const auto& point : points) {
        centroid[0] += point.x;
        centroid[1] += point.y;
        //centroid.rot += point.rot;
    }
    centroid[0] /= points.size();
    centroid[1] /= points.size();
    //centroid.rot /= points.size();
    return Point{centroid[0], centroid[1], 0.0};
}

// Function to rotate a set of points around the origin
void rotatePoints(vector<Point>& points, double angle) {
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);

    for (auto& point : points) {
        double newX = point.x * cosAngle - point.y * sinAngle;
        double newY = point.x * sinAngle + point.y * cosAngle;
        point.x = newX;
        point.y = newY;
        point.rot += angle*1000;
    }
}

// Function to compare two points within a threshold
bool comparePoints(const Point& p1, const Point& p2) {
    // Euclidean distance threshold
    // double distance = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    // return distance <= GMAP_TRANS_ERR_THRESHOLD;

    // manhattan distance threshold
    return (abs(p1.x - p2.x) <= GMAP_TRANS_ERR_THRESHOLD && 
            abs(p1.y - p2.y) <= GMAP_TRANS_ERR_THRESHOLD    );
}

// simple binary search
size_t binary_search_simple(const vector<Point>& points, const Point& point, bool (*compare)(const Point&, const Point&)) {
    size_t low = 0, high = points.size();
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (compare(points[mid], point))
            low = mid + 1;
        else
            high = mid;
    }
    return low;
}

// simple insertion sort
void insertion_sort(vector<Point>& points, bool (*compare)(const Point&, const Point&)) {
    for (size_t i = 1; i < points.size(); ++i) {
        Point key = points[i];
        size_t j = i;
        while (j > 0 && compare(points[j - 1], key)) {
            points[j] = points[j - 1];
            --j;
        }
        points[j] = key;
    }
}


void zeroCentroid(vector<Point>& points) {
    Point centroid = calculateCentroid(points);
    for (auto& point : points) {
        point.x -= centroid.x;
        point.y -= centroid.y;
    }
}


/** Function to compare two sets of points
 * @param set1 Smaller set of points
 * @param set2 Larger set of points (to compare with and preferably to merge into)
 * 
 * @return true if 75% of points in set1 have a match in set2 (manhattan GMAP_TRANS_ERR_THRESHOLD), false otherwise
 * 
 */
bool compareSets_verbose(vector<Point>& set1, vector<Point>& set2) {
    Point centroid1, centroid2;
    double rotDiff1 = 0.0, rotDiff2 = 0.0;

    // size
    cout << "Set1 size: " << set1.size() << endl;
    cout << "Set2 size: " << set2.size() << endl << endl << endl;

    insertion_sort(set2, compareTranlationX);
    insertion_sort(set1, compareTranlationX); // for verbose output

    { // centroid

        centroid1 = calculateCentroid(set1);
        centroid2 = calculateCentroid(set2);
        cout << "Centroid of set1: x: " << centroid1.x << ", y: " << centroid1.y << endl;
        cout << "Centroid of set2: x: " << centroid2.x << ", y: " << centroid2.y << endl << endl;

        // zero the centroid 
        zeroCentroid(set1);
        zeroCentroid(set2);

        cout << "Set1-2 after zeroing centroid" << endl << endl;

        // calculate centroid of both sets
        centroid1 = calculateCentroid(set1);
        centroid2 = calculateCentroid(set2);
        cout << "Re-Centroid of set1: x: " << centroid1.x << ", y: " << centroid1.y << endl;
        cout << "RE-Centroid of set2: x: " << centroid2.x << ", y: " << centroid2.y << endl << endl;

    }

    { // rotation calc
        rotDiff1 = 0.0, rotDiff2 = 0.0;
        // Calculate dominant rot for each set
        for (const auto& point : set1)  rotDiff1 += point.rot;
        rotDiff1 /= set1.size();

        for (const auto& point : set2)  rotDiff2 -= point.rot;
        rotDiff2 /= set2.size();

        cout << "Dominant rot of set1: " << rotDiff1 << endl;
        cout << "Dominant rot of set2: " << rotDiff2 << endl << endl;
    }

    { // do rotation
        cout << "Set1 before rot:" << endl << endl;
        for (const auto& point : set1) {
            cout << "[ " << point.x << ", " << point.y << "] ";
        }
        cout << endl << endl;

        // Rotate set1 to align dominant rots to set2
        rotatePoints(set1, (rotDiff2 - rotDiff1) / 1000 ); // mrad to rad

        cout << "Set1 after rot:" << endl;
        for (const auto& point : set1) {
            cout << "[ " << point.x << ", " << point.y << "] ";
        }
        cout << endl << endl;
    }

    insertion_sort(set2, compareTranlationX);
    insertion_sort(set1, compareTranlationX); // for verbose output
    
    { // rotation calc
        rotDiff1 = 0.0, rotDiff2 = 0.0;
        // Calculate dominant rot for each set
        for (const auto& point : set1)  rotDiff1 += point.rot;
        rotDiff1 /= set1.size();

        for (const auto& point : set2)  rotDiff2 -= point.rot;
        rotDiff2 /= set2.size();

        cout << "Dominant rot of set1: " << rotDiff1 << endl;
        cout << "Dominant rot of set2: " << rotDiff2 << endl << endl;
    }

    cout << "Set1 after trans + rot + sort + zeroCentroid" << endl;
    for (const auto& point : set1) {
        cout << "[ " << point.x << ", " << point.y << "] ";
    }
    cout << endl << endl;
    cout << "Set2 after sort + zeroCentroid" << endl;
    for (const auto& point : set2) {
        cout << "[ " << point.x << ", " << point.y << "] ";
    }
    cout << endl << endl;

    int points_matched = 0;

    // Check if corresponding points in both sets are equal within the threshold
    for (size_t i = 0; i < set1.size(); ++i) {
        size_t j;
        for (j=0; j < set2.size(); ++j)
            
            if (comparePoints(set1[i], set2[j])) {
                //cout << "Points matched" << endl << endl;
                points_matched++;
                break;
            }
    }

    cout << "Number of matched points: " << points_matched << endl << endl;

    // per point match verbose
    for (int i=0; i<set1.size(); i++) {
        cout << "Point: [" << set1[i].x << ", " << set1[i].y << "]" << endl;
        cout << "Point: [" << set2[i].x << ", " << set2[i].y << "]\t";
        if (comparePoints(set1[i], set2[i])) {
            cout << "Matched" << endl;
        } else {
            cout << "Not matched" << endl;
        }
        cout << "Diff : [" << set1[i].x - set2[i].x << ", " << set1[i].y - set2[i].y << "]" << endl << endl;    
    }

    // If the number of matched points is less than the threshold, sets are not equal
    if (points_matched < set1.size() * 0.75)
        return false;
    else
        return true;
}


int main() {
    // Example usage
    std::vector<Point> set2 = {{ 20569 ,  -21606 ,  1165 }, { 34510 ,  -20092 ,  1165 }, { 20630 ,  -23824 ,  1165 }, { 32977 ,  -18064 ,  1165 }, { 27233 ,  -28062 ,  1165 }, { 29131 ,  -28433 ,  1165 }, { 30306 ,  -15659 ,  1165 }, { 22517 ,  -21897 ,  1165 }, { 19757 ,  -22962 ,  1165 }, { 21522 ,  -23137 ,  1165 }, { 16214 ,  -15836 ,  1165 }, { 34154 ,  -11754 ,  1165 }, { 19977 ,  -23277 ,  1165 }, { 22383 ,  -27862 ,  1165 }, { 31373 ,  -19719 ,  1165 }, { 19230 ,  -18565 ,  1165 }, { 32134 ,  -12409 ,  1165 }, { 24885 ,  -12110 ,  1165 }, { 31397 ,  -13955 ,  1165 }, { 29281 ,  -22532 ,  1165 }, { 32095 ,  -27352 ,  1165 }, { 29130 ,  -24936 ,  1165 }, { 27540 ,  -18518 ,  1165 }, { 34954 ,  -13395 ,  1165 }, { 24276 ,  -14671 ,  1165 }, { 16772 ,  -28610 ,  1165 }, { 15864 ,  -19386 ,  1165 }, { 17212 ,  -18319 ,  1165 }, { 21294 ,  -24103 ,  1165 }, { 23508 ,  -12680 ,  1165 }, { 14966 ,  -24713 ,  1165 }, { 25999 ,  -23453 ,  1165 }, { 22753 ,  -27156 ,  1165 }, { 32250 ,  -25320 ,  1165 }, };
    //{{ -7257 ,  30022 ,  3764 }, { -19978 ,  35924 ,  3764 }, { -6164 ,  31953 ,  3764 }, { -19712 ,  33396 ,  3764 }, { -9631 ,  38992 ,  3764 }, { -11065 ,  40290 ,  3764 }, { -18667 ,  29957 ,  3764 }, { -8775 ,  31277 ,  3764 }, { -5862 ,  30764 ,  3764 }, { -7283 ,  31825 ,  3764 }, { -6507 ,  22832 ,  3764 }, { -23978 ,  28600 ,  3764 }, { -5888 ,  31148 ,  3764 }, { -5581 ,  36316 ,  3764 }, { -17484 ,  33985 ,  3764 }, { -7681 ,  26727 ,  3764 }, { -21910 ,  28118 ,  3764 }, { -15857 ,  24119 ,  3764 }, { -20481 ,  29061 ,  3764 }, { -14240 ,  35314 ,  3764 }, { -14161 ,  40895 ,  3764 }, { -12869 ,  37295 ,  3764 }, { -14822 ,  30977 ,  3764 }, { -23816 ,  30418 ,  3764 }, { -14013 ,  25997 ,  3764 }, { -389 ,  34060 ,  3764 }, { -4375 ,  25692 ,  3764 }, { -6080 ,  25474 ,  3764 }, { -6589 ,  32535 ,  3764 }, { -14383 ,  23896 ,  3764 }, { -855 ,  29790 ,  3764 }, { -10954 ,  34408 ,  3764 }, { -6262 ,  35903 ,  3764 }, { -15343 ,  39234 ,  3764 }, };
    //{{ 13899 ,  -31127 ,  3084 }, { -5108 ,  -30800 ,  3084 }, { 1861 ,  -28988 ,  3084 }, { -4608 ,  -27148 ,  3084 }, { 823 ,  -22529 ,  3084 }, { -2046 ,  -22066 ,  3084 }, { -1203 ,  -37436 ,  3084 }, { 13653 ,  -31336 ,  3084 }, { 9777 ,  -28228 ,  3084 }, { 8256 ,  -33511 ,  3084 }, { 4306 ,  -37392 ,  3084 }, { 4891 ,  -27393 ,  3084 }, { -3407 ,  -25588 ,  3084 }, { 10558 ,  -23596 ,  3084 }, { 9834 ,  -34681 ,  3084 }, { 15333 ,  -31661 ,  3084 }, { 8008 ,  -34426 ,  3084 }, { -5427 ,  -24872 ,  3084 }, { 1107 ,  -19257 ,  3084 }, { -7281 ,  -27871 ,  3084 }, { 14229 ,  -32280 ,  3084 }, { 1213 ,  -32962 ,  3084 }, { -10559 ,  -28764 ,  3084 }, { -3073 ,  -33424 ,  3084 }, { 8652 ,  -32279 ,  3084 }, { 1598 ,  -22100 ,  3084 }, { 12864 ,  -30816 ,  3084 }, { 14999 ,  -31598 ,  3084 }, { 2384 ,  -25056 ,  3084 }, { 3637 ,  -33361 ,  3084 }, { 3223 ,  -19932 ,  3084 }, { -3720 ,  -23075 ,  3084 }, { 3196 ,  -34765 ,  3084 }, { 8592 ,  -37047 ,  3084 }, { 10048 ,  -29195 ,  3084 }, { 10206 ,  -36882 ,  3084 }, };

    std::vector<Point> set1 = {{ 20569 ,  -21606 ,  1165 }, { 34510 ,  -20092 ,  1165 }, { 20630 ,  -23824 ,  1165 }, { 32977 ,  -18064 ,  1165 }, { 27233 ,  -28062 ,  1165 }, { 29131 ,  -28433 ,  1165 }, { 30306 ,  -15659 ,  1165 }, { 22517 ,  -21897 ,  1165 }, { 19757 ,  -22962 ,  1165 }, { 21522 ,  -23137 ,  1165 }, { 16214 ,  -15836 ,  1165 }, { 34154 ,  -11754 ,  1165 }, { 19977 ,  -23277 ,  1165 }, { 22383 ,  -27862 ,  1165 }, { 31373 ,  -19719 ,  1165 }, { 19230 ,  -18565 ,  1165 }, { 32134 ,  -12409 ,  1165 }, { 24885 ,  -12110 ,  1165 }, { 31397 ,  -13955 ,  1165 }, { 29281 ,  -22532 ,  1165 }, { 32095 ,  -27352 ,  1165 }, { 29130 ,  -24936 ,  1165 }, { 27540 ,  -18518 ,  1165 }, { 34954 ,  -13395 ,  1165 }, { 24276 ,  -14671 ,  1165 }, { 16772 ,  -28610 ,  1165 }, { 15864 ,  -19386 ,  1165 }, { 17212 ,  -18319 ,  1165 }, { 21294 ,  -24103 ,  1165 }, { 23508 ,  -12680 ,  1165 }, { 14966 ,  -24713 ,  1165 }, { 25999 ,  -23453 ,  1165 }, { 22753 ,  -27156 ,  1165 }, { 32250 ,  -25320 ,  1165 }, };
    //{{ -12265 ,  -32474 ,  2374 }, { -26467 ,  -19836 ,  2374 }, { -20000 ,  -23005 ,  2374 }, { -23707 ,  -17392 ,  2374 }, { -16577 ,  -17430 ,  2374 }, { -18451 ,  -15208 ,  2374 }, { -27831 ,  -27414 ,  2374 }, { -12587 ,  -32472 ,  2374 }, { -13501 ,  -27589 ,  2374 }, { -18098 ,  -30604 ,  2374 }, { -23624 ,  -30972 ,  2374 }, { -16662 ,  -23770 ,  2374 }, { -21779 ,  -16992 ,  2374 }, { -9889 ,  -24585 ,  2374 }, { -17664 ,  -32520 ,  2374 }, { -11525 ,  -33814 ,  2374 }, { -18883 ,  -31136 ,  2374 }, { -22844 ,  -15132 ,  2374 }, { -14228 ,  -15134 ,  2374 }, { -26205 ,  -16198 ,  2374 }, { -12766 ,  -33564 ,  2374 }, { -23081 ,  -25596 ,  2374 }, { -29273 ,  -14739 ,  2374 }, { -26634 ,  -23152 ,  2374 }, { -16995 ,  -29927 ,  2374 }, { -15709 ,  -17610 ,  2374 }, { -12847 ,  -31563 ,  2374 }, { -11738 ,  -33548 ,  2374 }, { -17040 ,  -20364 ,  2374 }, { -21503 ,  -27479 ,  2374 }, { -13064 ,  -17025 ,  2374 }, { -20378 ,  -14882 ,  2374 }, { -22753 ,  -28256 ,  2374 }, { -20148 ,  -33504 ,  2374 }, { -13926 ,  -28498 ,  2374 }, { -18817 ,  -34431 ,  2374 }, };

    if (compareSets_verbose(set1, set2)) {
        cout << "Sets are equal within the threshold." << endl << endl;
    } else {
        cout << "Sets are not equal within the threshold." << endl << endl;
    }

    return 0;
}
