/*
 * Hydrogen
 * Copyright(c) 2015 by Sacha Delanoue
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef LILYPOND_H
#define LILYPOND_H

class QString;

namespace H2Core {

class Song;

/// A class to convert a Hydrogen song to LilyPond format
class LilyPond {
public:
	LilyPond();

	/*
	 * Retreive all needed data from an Hydrogen song
	 * @param song the Hydrogen song to convert
	 */
	void extractData( const Song &song );

	/*
	 * Write the LilyPond format into a file
	 * @param sFilename name of output file
	 */
	void write( const QString &sFilename ) const;
};
}

#endif // LILYPOND_H
