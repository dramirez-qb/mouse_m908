/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "mouse_m715.h"

mouse_m715::rd_profile mouse_m715::get_profile(){
	return _s_profile;
}

uint8_t mouse_m715::get_scrollspeed( rd_profile profile ){
	return _s_scrollspeeds[profile];
}

mouse_m715::rd_lightmode mouse_m715::get_lightmode( rd_profile profile ){
	return _s_lightmodes[profile];
}

void mouse_m715::get_color( rd_profile profile, std::array<uint8_t, 3> &color ){
	color = _s_colors[profile];
}

uint8_t mouse_m715::get_brightness( rd_profile profile ){
	return _s_brightness_levels[profile];
}

uint8_t mouse_m715::get_speed( rd_profile profile ){
	return _s_speed_levels[profile];
}

bool mouse_m715::get_dpi_enable( rd_profile profile, int level ){
	
	// check DPI level bounds
	if( level < _c_level_min || level > _c_level_max )
		return false;
	
	return _s_dpi_enabled[profile][level];
}

int mouse_m715::get_dpi( rd_profile profile, int level, std::array<uint8_t, 2>& dpi ){
	
	// check DPI level bounds
	if( level < _c_level_min || level > _c_level_max )
		return 1;
	
	dpi[0] = _s_dpi_levels[profile][level][0];
	dpi[1] = _s_dpi_levels[profile][level][1];
	return 0;
}

mouse_m715::rd_report_rate mouse_m715::get_report_rate( rd_profile profile ){
	return _s_report_rates[profile];
}

uint8_t mouse_m715::get_macro_repeat( int macro_number ){
	
	//check if macro_number is valid
	if( macro_number < 1 || macro_number > 15 ){
		return 1;
	}
	
	return _s_macro_repeat[macro_number];
}

int mouse_m715::get_key_mapping_raw( mouse_m715::rd_profile profile, int key, std::array<uint8_t, 4>& mapping ){
	
	// valid key ?
	if( _c_button_names[key] == "" )
		return 1;
	
	mapping[0] = _s_keymap_data[profile][key][0];
	mapping[1] = _s_keymap_data[profile][key][1];
	mapping[2] = _s_keymap_data[profile][key][2];
	mapping[3] = _s_keymap_data[profile][key][3];
	
	return 0;
}

int mouse_m715::get_key_mapping( mouse_m715::rd_profile profile, int key, std::string& mapping ){
	
	// valid key ?
	if( _c_button_names[key] == "" )
		return 1;
	
	uint8_t b1 = _s_keymap_data[profile][key][0];
	uint8_t b2 = _s_keymap_data[profile][key][1];
	uint8_t b3 = _s_keymap_data[profile][key][2];
	uint8_t b4 = _s_keymap_data[profile][key][3];
	bool found_name = false;
	
	mapping = "";
	
	// fire button
	if( b1 == 0x99 ){
		
		mapping += "fire:";
		
		// button
		if( b2 == 0x81 )
			mapping += "mouse_left:";
		else if( b2 == 0x82 )
			mapping += "mouse_right:";
		else if( b2 == 0x84 )
			mapping += "mouse_middle:";
		else{
			
			// iterate over _c_keyboard_key_values
			for( auto keycode : _c_keyboard_key_values ){
				
				if( keycode.second == b2 ){
					
					mapping += keycode.first;
					break;
					
				}
				
			}
			mapping += ":";
		}
		
		// repeats
		mapping += std::to_string( (int)b3 ) + ":";
		
		// delay
		mapping += std::to_string( (int)b4 );
		
		found_name = true;
		
	// keyboard key
	} else if( b1 == 0x90 ){
		
		// iterate over _c_keyboard_key_values
		for( auto keycode : _c_keyboard_key_values ){
			
			if( keycode.second == b3 ){
				
				mapping += keycode.first;
				found_name = true;
				break;
				
			}
			
		}
		
	// modifiers + keyboard key
	} else if( b1 == 0x8f ){
		
		// iterate over _c_keyboard_modifier_values
		for( auto modifier : _c_keyboard_modifier_values ){
			
			if( modifier.second & b2 ){
				mapping += modifier.first;
			}
			
		}
		
		// iterate over _c_keyboard_key_values
		for( auto keycode : _c_keyboard_key_values ){
			
			if( keycode.second == b3 ){
				
				mapping += keycode.first;
				found_name = true;
				break;
				
			}
			
		}
		
	} else{ // mousebutton or special function ?
		
		// iterate over _c_keycodes
		for( auto keycode : _c_keycodes ){
			
			if( keycode.second[0] == b1 &&
				keycode.second[1] == b2 && 
				keycode.second[2] == b3 ){
				
				mapping += keycode.first;
				found_name = true;
				break;
				
			}
			
		}
		
	}
	
	if( !found_name ){
		mapping += "unknown, please report as bug: (dec) ";
		mapping += " " + std::to_string( (int)b1 ) + " ";
		mapping += " " + std::to_string( (int)b2 ) + " ";
		mapping += " " + std::to_string( (int)b3 ) + " ";
		mapping += " " + std::to_string( (int)b4 );
	}
	
	return 0;
}

int mouse_m715::get_macro_raw( int number, std::array<uint8_t, 256>& macro ){
	
	//check if macro_number is valid
	if( number < 1 || number > 15 )
		return 1;
	
	std::copy( _s_macro_data[number-1].begin(), _s_macro_data[number-1].end(), macro.begin() );
	
	return 0;
}

int mouse_m715::get_macro( int number, std::string& macro ){
	
	std::stringstream output;
	
	// macro undefined?
	if( _s_macro_data[number-1][8] == 0 && _s_macro_data[number-1][9] == 0 && _s_macro_data[number-1][10] == 0 )
		return 0;
	
	for( long unsigned int j = 8; j < _s_macro_data[number-1].size(); ){
		
		// failsafe
		if( j >= _s_macro_data[number-1].size() )
			break;
		
		if( _s_macro_data[number-1][j] == 0x81 ){ // mouse button down
			
			if( _s_macro_data[number-1][j] == 0x01 )
				output << "down\tmouse_left\n";
			else if( _s_macro_data[number-1][j] == 0x02 )
				output << "down\tmouse_right\n";
			else if( _s_macro_data[number-1][j] == 0x04 )
				output << "down\tmouse_middle\n";
			else{
				output << "unknown, please report as bug: ";
				output << std::hex << (int)_s_macro_data[number-1][j] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+1] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+2];
				output << std::dec << "\n";
			}
			
		} else if( _s_macro_data[number-1][j] == 0x01 ){ // mouse button up
			
			if( _s_macro_data[number-1][j] == 0x01 )
				output << "up\tmouse_left\n";
			else if( _s_macro_data[number-1][j] == 0x02 )
				output << "up\tmouse_right\n";
			else if( _s_macro_data[number-1][j] == 0x04 )
				output << "up\tmouse_middle\n";
			else{
				output << "unknown, please report as bug: ";
				output << std::hex << (int)_s_macro_data[number-1][j] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+1] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+2];
				output << std::dec << "\n";
			}
			
		} else if( _s_macro_data[number-1][j] == 0x84 ){ // keyboard key down
			
			bool found_name = false;
			
			// iterate over _c_keyboard_key_values
			for( auto keycode : _c_keyboard_key_values ){
				
				if( keycode.second == _s_macro_data[number-1][j+1] ){
					
					output << "down\t" << keycode.first << "\n";
					found_name = true;
					break;
					
				}
				
			}
			
			if( !found_name ){
				output << "unknown, please report as bug: ";
				output << std::hex << (int)_s_macro_data[number-1][j] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+1] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+2];
				output << std::dec << "\n";
			}
			
		} else if( _s_macro_data[number-1][j] == 0x04 ){ // keyboard key up
			
			bool found_name = false;
			
			// iterate over _c_keyboard_key_values
			for( auto keycode : _c_keyboard_key_values ){
				
				if( keycode.second == _s_macro_data[number-1][j+1] ){
					
					output << "up\t" << keycode.first << "\n";
					found_name = true;
					break;
					
				}
				
			}
			
			if( !found_name ){
				output << "unknown, please report as bug: ";
				output << std::hex << (int)_s_macro_data[number-1][j] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+1] << " ";
				output << std::hex << (int)_s_macro_data[number-1][j+2];
				output << std::dec << "\n";
			}
			
		} else if( _s_macro_data[number-1][j] == 0x06 ){ // delay
			
			output << "delay\t" << (int)_s_macro_data[number-1][j+1] << "\n";
			
		} else if( _s_macro_data[number-1][j] == 0x00 ){ // padding
			
			j++;
			
		} else{
			output << "unknown, please report as bug: ";
			output << std::hex << (int)_s_macro_data[number-1][j] << " ";
			output << std::hex << (int)_s_macro_data[number-1][j+1] << " ";
			output << std::hex << (int)_s_macro_data[number-1][j+2];
			output << std::dec << "\n";
		}
		
		// increment
		j+=3;
		
	}
	
	macro = output.str();
	return 0;
}
