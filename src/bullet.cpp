/* This file is part of Geo
 *
 * Copyright (C) 2011 Trevor Dodds <@gmail.com trev.dodds>
 *
 * Geo is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * Geo is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Geo.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "bullet.h"

Bullet::Bullet()
{
  active = false;
}

Bullet::~Bullet()
{
}

void Bullet::setActive(bool b)
{
  active = b;
}

bool Bullet::getActive()
{
  return active;
}

