// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
const unsigned char guImage_go_down[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x02,
  0x20, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0xad, 0x53, 0xcf, 0x4f, 0x13,
  0x51, 0x10, 0xfe, 0x5e, 0x77, 0xdb, 0x42, 0x7f, 0x20, 0x56, 0x83, 0xb5,
  0x15, 0x22, 0x26, 0x18, 0x13, 0x51, 0x23, 0x7a, 0x22, 0x92, 0xaa, 0x07,
  0x13, 0xf5, 0x6c, 0x42, 0x8c, 0x37, 0xe3, 0xc9, 0x84, 0x0b, 0xfe, 0x01,
  0xde, 0x0c, 0x46, 0xce, 0xde, 0xf5, 0xc4, 0x05, 0xaf, 0x78, 0x12, 0x03,
  0x6e, 0x22, 0x04, 0x4c, 0x68, 0x23, 0xa9, 0x21, 0x5a, 0xa8, 0x24, 0xad,
  0xb4, 0xc8, 0xae, 0xee, 0xb6, 0xdb, 0xb7, 0x6f, 0xc6, 0x53, 0x9b, 0x75,
  0xc1, 0x78, 0xd0, 0x79, 0x99, 0xbc, 0x1f, 0xf9, 0xe6, 0xcb, 0x37, 0x33,
  0x6f, 0x80, 0x7f, 0x34, 0x11, 0x7c, 0x18, 0x7b, 0x94, 0xbc, 0xda, 0xd7,
  0x33, 0x94, 0x23, 0x22, 0x28, 0x56, 0x60, 0x56, 0x60, 0x10, 0x1a, 0xd2,
  0x92, 0x2e, 0x55, 0xa7, 0x17, 0xa7, 0x94, 0xeb, 0xc7, 0xeb, 0x41, 0x82,
  0x23, 0x89, 0xa1, 0xdc, 0xf8, 0xad, 0x7b, 0x8f, 0xbf, 0x3b, 0x15, 0xb8,
  0x5e, 0x13, 0xae, 0xd7, 0x84, 0xf4, 0x5a, 0xb0, 0xf7, 0x9a, 0x3f, 0xe6,
  0xd7, 0x66, 0x9f, 0x03, 0x7f, 0x21, 0x10, 0x10, 0x70, 0x5a, 0x16, 0x4a,
  0x7b, 0x2b, 0x70, 0xa4, 0x09, 0xbb, 0x65, 0xc1, 0x95, 0x12, 0x27, 0xf4,
  0xcb, 0xe0, 0xfd, 0x82, 0x11, 0x3a, 0x28, 0x2f, 0x6e, 0x2f, 0x06, 0xc0,
  0x0c, 0x22, 0xfe, 0x63, 0x0d, 0x3a, 0x0a, 0x46, 0x27, 0xf5, 0x18, 0x04,
  0xc0, 0x4c, 0x11, 0x66, 0x02, 0x31, 0xa1, 0xbd, 0x4b, 0x25, 0x01, 0x06,
  0x40, 0x1c, 0x1b, 0x9d, 0xd4, 0x5d, 0x00, 0xd2, 0x98, 0xf6, 0x64, 0x87,
  0x60, 0x74, 0x52, 0x8f, 0x66, 0x7a, 0x87, 0x5f, 0x8f, 0x9d, 0xbf, 0x79,
  0x0e, 0x82, 0xba, 0x09, 0x04, 0x86, 0x07, 0x25, 0x5a, 0x70, 0xc8, 0x04,
  0x6b, 0x61, 0xa4, 0x52, 0xa9, 0xc4, 0xf5, 0x8b, 0x77, 0xd6, 0xab, 0xb5,
  0x4a, 0xeb, 0xe3, 0xe6, 0xc2, 0x43, 0x00, 0x33, 0x9d, 0x14, 0x8c, 0x69,
  0xcf, 0x55, 0xac, 0x26, 0xb4, 0x88, 0x74, 0x38, 0x51, 0x8f, 0x94, 0xac,
  0x65, 0xd8, 0xb4, 0x8b, 0x3d, 0x2a, 0x03, 0x51, 0x07, 0xa1, 0x6e, 0x13,
  0x6b, 0xd6, 0x9c, 0x08, 0x25, 0xed, 0xb8, 0xd5, 0xdc, 0x99, 0x15, 0x21,
  0xf1, 0xaa, 0xad, 0x5c, 0xeb, 0x54, 0xff, 0x52, 0xbd, 0x6a, 0x5a, 0xee,
  0xd6, 0x60, 0xe6, 0xf4, 0xed, 0x1d, 0xef, 0x4b, 0xb8, 0xee, 0x7d, 0x82,
  0x1e, 0x61, 0xe8, 0x51, 0x01, 0x3d, 0xac, 0xa1, 0xbf, 0xe7, 0x02, 0xd7,
  0xb6, 0x6c, 0xe3, 0xf3, 0x76, 0xe1, 0xfe, 0xc2, 0xd3, 0xa6, 0xbd, 0x8f,
  0xa0, 0x6c, 0x30, 0x52, 0x23, 0x66, 0xb1, 0xd1, 0x68, 0x85, 0x32, 0xe9,
  0xec, 0x95, 0x9f, 0x5a, 0x45, 0xd3, 0xbb, 0x14, 0x22, 0x5d, 0x1a, 0xfa,
  0xe2, 0x67, 0xa1, 0x6a, 0xf1, 0x72, 0xbe, 0xb8, 0x3a, 0x3e, 0xff, 0xc4,
  0x2a, 0xfb, 0x8b, 0xa8, 0xf9, 0x2f, 0x9b, 0x86, 0xe4, 0xe4, 0xc8, 0xee,
  0x52, 0x98, 0xe2, 0xc3, 0x27, 0xfb, 0x07, 0xcf, 0x34, 0xf4, 0xaa, 0xe8,
  0x89, 0xa7, 0x71, 0xc8, 0x19, 0xb4, 0x97, 0x57, 0x3e, 0x3c, 0x30, 0x9d,
  0xda, 0xbb, 0x6d, 0xe3, 0xf7, 0x8e, 0x68, 0xc1, 0xb6, 0x6c, 0xaf, 0x4a,
  0x8a, 0x0e, 0x98, 0xc6, 0xd1, 0x78, 0xf6, 0xda, 0xb1, 0x74, 0xe6, 0x78,
  0x2f, 0x0d, 0xc8, 0xa5, 0xc5, 0xfc, 0x54, 0xfe, 0x6d, 0xe9, 0x45, 0xe1,
  0x25, 0x51, 0x10, 0xaf, 0x05, 0xce, 0x5d, 0x2c, 0x91, 0xf8, 0xba, 0xe2,
  0x0a, 0x4a, 0xd5, 0x8b, 0xd9, 0xc3, 0xa7, 0x6e, 0x6c, 0xac, 0x97, 0xdf,
  0xbf, 0x99, 0x29, 0x3c, 0x2b, 0x2f, 0x50, 0x08, 0x40, 0xd8, 0x17, 0xa3,
  0x82, 0xb3, 0xd0, 0x06, 0x44, 0x01, 0x44, 0xf5, 0x24, 0x22, 0x23, 0x77,
  0x63, 0xb9, 0xdd, 0x4d, 0xb7, 0xb8, 0x31, 0xa7, 0xbe, 0x01, 0x90, 0x00,
  0x5c, 0x9f, 0xab, 0x03, 0x87, 0xc9, 0x37, 0x64, 0xc2, 0xf7, 0x53, 0x39,
  0xe0, 0xff, 0xcf, 0x7e, 0x01, 0xe6, 0xb4, 0x0c, 0xeb, 0x37, 0x45, 0xfc,
  0x22, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60,
  0x82
};