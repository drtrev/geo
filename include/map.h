/* This file is part of Map - A tool for viewing 2D map data using Motlab (a
 * heterogeneous collaborative visualization library).
 *
 * Copyright (C) 2009 Trevor Dodds <trev@comp.leeds.ac.uk>
 *
 * Map is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Map is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Map.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

/**
 * Define a polygon by giving coordinates for inner and outer boundaries, and colour.
 */
// typedef only needed in C
typedef struct {
  std::vector < std::vector<double> > outer;
  std::vector < std::vector<double> > inner;
  int colour;
} MapPolygon;

class Graphics; class Outverbose;

/**
 * The map class.
 *
 * Stores a map object, which can be made up of polygons (e.g. GML data).
 */
class Map {
  private:
    Outverbose* out; /**< The instance of the output object. */
    Graphics* graphics; /**< The graphics object. */
    std::vector < std::vector < std::vector<double> > > linestrip; /**< The polylines used for rendering. */
    std::vector < MapPolygon > polygon; /**< The polygons used for rendering. */

    /** \var x
     *  The x coordinate. */
    /** \var y
     *  The y coordinate. */
    /** \var accelX
     *  The x acceleration. */
    /** \var accelY
     *  The y acceleration. */
    /** \var power
     *  The power of the object. */
    /** \var friction
     *  The object's friction. */
    /** \var speedX
     *  The horizontal speed. */
    /** \var speedY
     *  The vertical speed. */
    /** \var minSpeed
     *  The minimum speed before it stops. */
    float x, y, accelX, accelY, power, friction, speedX, speedY, minSpeed;

    /** \var oldAccelX
     *  The previous x acceleration. */
    /** \var oldAccelY
     *  The previous y acceleration. */
    float oldAccelX, oldAccelY;

    float zoom; /**< The scale factor. */
    int polygonMax; /**< The maximum number of polygons. */

    //double highestSync; // temporary, for error checking

  public:
    Map(); /**< Constructor. */
    ~Map(); /**< Destructor. */

    /**
     * Initialisation.
     *
     * \param o the output object.
     * \param g the graphics object.
     * \param p the maximum number of polygons to load.
     */
    void init(Outverbose &o, Graphics &g, int p);

    void load(std::string); /**< Load a GML file. This is not implemented, but currently provides some example data. */
    
    /**
     * Split a string into tokens.
     *
     * \param str the string to divide.
     * \param tokens the resulting tokens.
     * \param delimiters the delimeters.
     */
    void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");

    void draw(bool thumb); /**< Render the map. \param thumb set to true if this is the thumbnail view, false otherwise. */

    void input(int in, double sync); /**< Deal with keyboard input. \param in the keys being pressed. \param sync the time since last frame. */

    void move(double sync); /**< Move the map. \param sync the time since last frame. */

    void setX(float nx); /**< Set the x coordinate. \param nx the new x coordinate. */
    void setY(float ny); /**< Set the y coordinate. \param ny the new y coordinate. */
    void setZoom(float nz); /**< Set the zoom factor. \param nz the new zoom factor. */
    float getX() const; /**< Get the x coordinate. \return the x coordinate. */
    float getY() const; /**< Get the y coordinate. \return the y coordinate. */
    float getZoom() const; /**< Get the zoom factor. \return the zoom factor. */
};

#endif
