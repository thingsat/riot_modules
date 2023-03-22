/*

Here's an example of a C function that checks if a point is inside a polygon.

This function takes a Point structure representing the point to check, and a Polygon structure representing the polygon to check against. The Polygon structure contains the number of points in the polygon and an array of Point structures representing the vertices of the polygon.

The function works by iterating over each edge of the polygon and checking if the ray from the point in the positive x-direction intersects with the edge. If the ray intersects with an odd number of edges, the point is inside the polygon; if it intersects with an even number of edges, the point is outside the polygon.

Note that this implementation assumes that the polygon is not self-intersecting and is oriented in a counter-clockwise direction. It may not work correctly for polygons with holes or for polygons that are not oriented consistently.

Author: ChatGPT 2023-03-22

*/

typedef struct {
    float x;
    float y;
} Point_t;

typedef struct {
    int num_points;
    Point_t *points;
} Polygon_t;

//#include "geofence_polygones.h"

/* assumes that the polygon is not self-intersecting and is oriented in a counter-clockwise */
int point_in_polygon(Point_t p, Polygon_t poly) {
    int i, j, c = 0;
    for (i = 0, j = poly.num_points - 1; i < poly.num_points; j = i++) {
        if (((poly.points[i].y > p.y) != (poly.points[j].y > p.y)) &&
            (p.x < (poly.points[j].x - poly.points[i].x) * (p.y - poly.points[i].y) /
            (poly.points[j].y - poly.points[i].y) + poly.points[i].x))
            c = !c;
    }
    return c;
}
