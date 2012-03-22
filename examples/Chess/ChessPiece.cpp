/*   Copyright 2012 Marios Fragkoulis
 *
 *   Licensed under the Apache License, Version 2.0
 *   (the "License");you may not use this file except in
 *   compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in
 *   writing, software distributed under the License is
 *   distributed on an "AS IS" BASIS.
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *   express or implied.
 *   See the License for the specific language governing
 *  permissions and limitations under the License.
 */

#include "ChessPiece.h"
#include "Position.h"
#include <vector>
#include <iostream>

using namespace std;

ChessPiece::ChessPiece(string n, string c) {
    name.assign(n);
    color.assign(c);
}

string ChessPiece::get_name() {
    return name;
}

string ChessPiece::get_color() {
    return color;
}
