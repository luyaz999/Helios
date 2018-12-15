/** \file "AerialLiDAR.cpp" Source file for LiDAR plug-in dealing with aerial scans.
    \author Brian Bailey

    Copyright (C) 2018  Brian Bailey

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*/

#include "AerialLiDAR.h"

using namespace std;
using namespace helios;

AerialScanMetadata::AerialScanMetadata( const helios::vec3 __center, const helios::vec2 __extent, const float __coneangle, const float __scandensity ){

  //Copy arguments into structure variables
  center = __center;
  extent = __extent;
  coneangle = __coneangle;
  scandensity = __scandensity;

}
 
AerialLiDARcloud::AerialLiDARcloud( void ){

  hitgridcellcomputed = false;
  printmessages = true;
  
}

AerialLiDARcloud::~AerialLiDARcloud( void ){
  
}

int AerialLiDARcloud::selfTest(void){

  return 0;
  
}

void AerialLiDARcloud::disableMessages( void ){
  printmessages = false;
}

void AerialLiDARcloud::enableMessages( void ){
  printmessages = true;
}

uint AerialLiDARcloud::getScanCount( void ){
  return scans.size();
}

void AerialLiDARcloud::addScan( const AerialScanMetadata newscan ){
  scans.push_back(newscan);
}

void AerialLiDARcloud::addHitPoint( const uint scanID, const helios::vec3 hit_xyz, helios::vec3 ray_origin ){

  vec3 direction_cart = hit_xyz-ray_origin;
  SphericalCoord direction_sphere = cart2sphere(direction_cart);

  //default color
  RGBcolor color = make_RGBcolor(1,0,0);

  //empty data
  std::map<std::string, float> data;
  
  addHitPoint( scanID, hit_xyz, direction_sphere, color, data );

}

void AerialLiDARcloud::addHitPoint( const uint scanID, const helios::vec3 hit_xyz, const helios::SphericalCoord direction ){

  //default color
  RGBcolor color = make_RGBcolor(1,0,0);

  //empty data
  std::map<std::string, float> data;
  
  addHitPoint( scanID, hit_xyz, direction, color, data );

}

void AerialLiDARcloud::addHitPoint( const uint scanID, const helios::vec3 hit_xyz, const helios::SphericalCoord direction, const std::map<std::string, float> data ){

  //default color
  RGBcolor color = make_RGBcolor(1,0,0);

  addHitPoint( scanID, hit_xyz, direction, color, data );
  
}

void AerialLiDARcloud::addHitPoint( const uint scanID, const helios::vec3 hit_xyz, const helios::SphericalCoord direction, const helios::RGBcolor color ){

  //empty data
  std::map<std::string, float> data;
  
  addHitPoint( scanID, hit_xyz, direction, color, data );
  
}

void AerialLiDARcloud::addHitPoint( const uint scanID, const helios::vec3 hit_xyz, const helios::SphericalCoord direction, const helios::RGBcolor color, const std::map<std::string, float> data ){

  //error checking
  if( scanID>=scans.size() ){
    cerr << "ERROR (addHitPoint): Hit point cannot be added to scan #" << scanID << " because there have only been " << scans.size() << " scans added." << endl;
    exit(EXIT_FAILURE);
  }

  AerialScanMetadata scan = scans.at(scanID);

  HitPoint hit( scanID, hit_xyz, direction, make_int2(0,0), color, data ); 

  hits.push_back( hit );

}

void AerialLiDARcloud::deleteHitPoint( const uint index ){

  if( index>=hits.size() ){
    cerr << "ERROR (deleteHitPoint): Hit point #" << index << " cannot be deleted from the scan because there have only been " << hits.size() << " hit points added." << endl;
    exit(EXIT_FAILURE);
  }
  
  HitPoint hit = hits.at(index);

  int scanID = hit.scanID;

  //erase from vector of hits (use swap-and-pop method)
  std::swap( hits.at(index), hits.back() );
  hits.pop_back();
  
}

uint AerialLiDARcloud::getHitCount( void ) const{
  return hits.size();
}

helios::vec3 AerialLiDARcloud::getScanCenter( const uint scanID ) const{
  if( scanID>=scans.size() ){
    cerr << "ERROR (getScanCenter): Cannot get center of scan #" << scanID << " because there have only been " << scans.size() << " scans added." << endl;
    exit(EXIT_FAILURE);
  }
  return scans.at(scanID).center;
}

helios::vec2 AerialLiDARcloud::getScanExtent( const uint scanID ) const{
  if( scanID>=scans.size() ){
    cerr << "ERROR (getScanExtent): Cannot get extent for scan #" << scanID << " because there have only been " << scans.size() << " scans added." << endl;
    exit(EXIT_FAILURE);
  }
  return scans.at(scanID).extent;
}

float AerialLiDARcloud::getScanConeAngle( const uint scanID ) const{
  if( scanID>=scans.size() ){
    cerr << "ERROR (getScanConeAngle): Cannot get cone angle for scan #" << scanID << " because there have only been " << scans.size() << " scans added." << endl;
    exit(EXIT_FAILURE);
  }
  return scans.at(scanID).coneangle;
}

float AerialLiDARcloud::getScanDensity( const uint scanID ) const{
  if( scanID>=scans.size() ){
    cerr << "ERROR (getScanDensity): Cannot get point density for scan #" << scanID << " because there have only been " << scans.size() << " scans added." << endl;
    exit(EXIT_FAILURE);
  }
  return scans.at(scanID).scandensity;
}

helios::vec3 AerialLiDARcloud::getHitXYZ( const uint index ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitXYZ): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  return hits.at(index).position;

}

helios::SphericalCoord AerialLiDARcloud::getHitRaydir( const uint index ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitRaydir): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  return hits.at(index).direction;

}

float AerialLiDARcloud::getHitData( const uint index, const char* label ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitData): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  std::map<std::string, float> hit_data = hits.at(index).data;
  if( hit_data.find(label) == hit_data.end() ){
    cerr << "ERROR (getHitData): Data value ``" << label << "'' does not exist." << std::endl;
    exit(EXIT_FAILURE);
  }

  return hit_data.at(label);

}

void AerialLiDARcloud::setHitData( const uint index, const char* label, const float value ){

  if( index>=hits.size() ){
    cerr << "ERROR (setHitScalarData): Hit point index out of bounds. Tried to set hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  hits.at(index).data[label] = value;
  
}

RGBcolor AerialLiDARcloud::getHitColor( const uint index ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitColor): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  return hits.at(index).color;

}

int AerialLiDARcloud::getHitScanID( const uint index ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitColor): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  return hits.at(index).scanID;
  
}

int AerialLiDARcloud::getHitGridCell( const uint index ) const{

  if( index>=hits.size() ){
    cerr << "ERROR (getHitGridCell): Hit point index out of bounds. Requesting hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }else if( hits.at(index).gridcell==-2 ){
    cerr << "WARNING (getHitGridCell): hit grid cell for point #" << index << " was never set.  Returning a value of `-1'.  Did you forget to call calculateHitGridCell[*] first?" << endl;
    return -1;
  }

  return hits.at(index).gridcell;
  
}

void AerialLiDARcloud::setHitGridCell( const uint index, const int cell ){

  if( index>=hits.size() ){
    cerr << "ERROR (setHitGridCell): Hit point index out of bounds. Tried to set hit #" << index << " but scan only has " << hits.size() << " hits." << endl;
    exit(EXIT_FAILURE);
  }

  hits.at(index).gridcell = cell;
  
}

void AerialLiDARcloud::coordinateShift( const helios::vec3 shift ){

  if( printmessages ){
    std::cout << "Performing coordinate shift..." << std::flush;
  }

  for( uint r=0; r<hits.size(); r++ ){
    hits.at(r).position = hits.at(r).position + shift;
  }

   if( printmessages ){
     std::cout << "done." << std::endl;
   }
  
}

void AerialLiDARcloud::addHitsToVisualizer( Visualizer* visualizer, const uint pointsize ) const{
  addHitsToVisualizer( visualizer, pointsize, "" );
}

void AerialLiDARcloud::addHitsToVisualizer( Visualizer* visualizer, const uint pointsize, const char* color_value ) const{

  if( printmessages && scans.size()==0 ){
    std::cout << "WARNING (addHitsToVisualizer): There are no scans in the point cloud, and thus there is no geometry to add...skipping." << std::endl;
    return;
  }

  //-- hit points --//
  float minval = 1e9;
  float maxval = -1e9;
  if( strcmp(color_value,"gridcell")==0 ){
    minval = 0;
    maxval = getGridCellCount()-1;
  }else if( strcmp(color_value,"")!=0 ){
    for( uint i=0; i<getHitCount(); i++ ){
      float data = getHitData(i,color_value);
      if( data<minval ){
	minval = data;
      }
      if( data>maxval ){
	maxval = data;
      }
    }
  }

  RGBcolor color;
  Colormap cmap = visualizer->getCurrentColormap();
  if( minval!=1e9 && maxval!=-1e9 ){
    cmap.setRange(minval,maxval);
  }

  for( uint i=0; i<getHitCount(); i++ ){
    
    if( strcmp(color_value,"")==0 ){
      color = getHitColor(i);
    }else if( strcmp(color_value,"gridcell")==0 ){
      color = cmap.query( getHitGridCell(i) );
    }else{
      float data = getHitData(i,color_value);
      color = cmap.query( data );
    }

    vec3 center = getHitXYZ(i);

    visualizer->addPoint( center, color, pointsize, Visualizer::COORDINATES_CARTESIAN );
    
  }

}

void AerialLiDARcloud::addGridToVisualizer( Visualizer* visualizer ) const{

  if( printmessages && scans.size()==0 ){
    std::cout << "WARNING (addGridToVisualizer): There are no scans in the point cloud, and thus there is no geometry to add...skipping." << std::endl;
    return;
  }

  float minval = 1e9;
  float maxval = -1e9;
  for( uint i=0; i<getGridCellCount(); i++ ){
    float data = getCellLeafAreaDensity(i);
    if( data<minval ){
      minval = data;
    }
    if( data>maxval ){
      maxval = data;
    }
  }

  Colormap cmap = visualizer->getCurrentColormap();
  if( minval!=1e9 && maxval!=-1e9 ){
    cmap.setRange(minval,maxval);
  }

  vec3 origin;
  for( uint i=0; i<getGridCellCount(); i++ ){

    //if( getCellLeafAreaDensity(i)==0 ){continue;}

    vec3 center = getCellCenter(i);

    vec3 anchor = getCellGlobalAnchor(i);

    SphericalCoord rotation = make_SphericalCoord(0,getCellRotation(i));

    center = rotatePointAboutLine( center, anchor, make_vec3(0,0,1), rotation.azimuth );
    vec3 size = getCellSize(i);
    
    //RGBAcolor color = make_RGBAcolor(RGB::green,0.5);

    RGBAcolor color = make_RGBAcolor(cmap.query(getCellLeafAreaDensity(i)),0.5);

    visualizer->addVoxelByCenter( center, size, rotation, color, Visualizer::COORDINATES_CARTESIAN );

    origin = origin + center/float(getGridCellCount());
 
  }

  vec3 boxmin, boxmax;
  getHitBoundingBox(boxmin,boxmax);

  float R = 2.f*sqrt( pow(boxmax.x-boxmin.x,2) + pow(boxmax.y-boxmin.y,2) + pow(boxmax.z-boxmin.z,2) );
  
}
  
void AerialLiDARcloud::getHitBoundingBox( helios::vec3& boxmin, helios::vec3& boxmax ) const{

  if( printmessages && hits.size()==0 ){
    std::cout << "WARNING (getHitBoundingBox): There are no hit points in the point cloud, cannot determine bounding box...skipping." << std::endl;
    return;
  }

  boxmin = make_vec3( 1e6, 1e6, 1e6 );
  boxmax = make_vec3( -1e6, -1e6, -1e6 );

  for( std::size_t i=0; i<hits.size(); i++ ){
 
    vec3 xyz = getHitXYZ(i);

    if( xyz.x<boxmin.x ){
      boxmin.x = xyz.x;
    }
    if( xyz.x>boxmax.x ){
      boxmax.x = xyz.x;
    }
    if( xyz.y<boxmin.y ){
      boxmin.y = xyz.y;
    }
    if( xyz.y>boxmax.y ){
      boxmax.y = xyz.y;
    }
    if( xyz.z<boxmin.z ){
      boxmin.z = xyz.z;
    }
    if( xyz.z>boxmax.z ){
      boxmax.z = xyz.z;
    }

  }
    
}

void AerialLiDARcloud::getGridBoundingBox( helios::vec3& boxmin, helios::vec3& boxmax ) const{

  if( printmessages && getGridCellCount()==0 ){
    std::cout << "WARNING (getGridBoundingBox): There are no grid cells in the point cloud, cannot determine bounding box...skipping." << std::endl;
    return;
  }

  boxmin = make_vec3( 1e6, 1e6, 1e6 );
  boxmax = make_vec3( -1e6, -1e6, -1e6 );

  std::size_t count = 0;
  for( uint c=0; c<getGridCellCount(); c++ ){
 
    vec3 center = getCellCenter(c);
    vec3 size = getCellSize(c);
    vec3 cellanchor = getCellGlobalAnchor(c);
    float rotation = getCellRotation(c);

    vec3 xyz_min = center - 0.5f*size;
    xyz_min = rotatePointAboutLine(xyz_min,cellanchor,make_vec3(0,0,1),rotation);
    vec3 xyz_max = center + 0.5f*size;
    xyz_max = rotatePointAboutLine(xyz_max,cellanchor,make_vec3(0,0,1),rotation);

    if( xyz_min.x<boxmin.x ){
      boxmin.x = xyz_min.x;
    }
    if( xyz_max.x>boxmax.x ){
      boxmax.x = xyz_max.x;
    }
    if( xyz_min.y<boxmin.y ){
      boxmin.y = xyz_min.y;
    }
    if( xyz_max.y>boxmax.y ){
      boxmax.y = xyz_max.y;
    }
    if( xyz_min.z<boxmin.z ){
      boxmin.z = xyz_min.z;
    }
    if( xyz_max.z>boxmax.z ){
      boxmax.z = xyz_max.z;
    }

  }

  
}

void AerialLiDARcloud::reflectanceFilter( const float minreflectance ){

  std::size_t delete_count = 0;
  for( std::size_t r=0; r<getHitCount(); r++ ){
    if( hits.at(r).data.find("reflectance") != hits.at(r).data.end() ){
      float R = getHitData(r,"reflectance");
      if( R<minreflectance ){
	deleteHitPoint(r);
	delete_count++;
      }
    }
  }

  if( printmessages ){
    std::cout << "Removed " << delete_count << " hit points based on reflectance filter." << std::endl;
  }
  
}

void AerialLiDARcloud::scalarFilter( const char* scalar_field, const float threshold, const char* comparator ){

  std::size_t delete_count = 0;
  for( std::size_t r=0; r<getHitCount(); r++ ){
    if( hits.at(r).data.find(scalar_field) != hits.at(r).data.end() ){
      float R = getHitData(r,scalar_field);
      if( strcmp(comparator,"<")==0 ){
	if( R<threshold ){
	  deleteHitPoint(r);
	  delete_count++;
	}
      }else if( strcmp(comparator,">")==0 ){
	if( R>threshold ){
	  deleteHitPoint(r);
	  delete_count++;
	}
      }else if( strcmp(comparator,"=")==0 ){
	if( R==threshold ){
	  deleteHitPoint(r);
	  delete_count++;
	}
      }
    }
  }

  if( printmessages ){
    std::cout << "Removed " << delete_count << " hit points based on scalar filter." << std::endl;
  }
  
}

uint AerialLiDARcloud::getGridCellCount( void ) const{
  return grid_cells.size();
}

void AerialLiDARcloud::addGridCell( const helios::vec3 center, const helios::vec3 size ){
  addGridCell(center,center,size,0.f);
}

void AerialLiDARcloud::addGridCell( const helios::vec3 center, const helios::vec3 global_anchor, const helios::vec3 size, const float rotation ){

  GridCell newcell( center, global_anchor, size, rotation );

  grid_cells.push_back(newcell);
  
}

helios::vec3 AerialLiDARcloud::getCellCenter( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellCenter): grid cell index out of range.  Requested center of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  return grid_cells.at(index).center;
  
}

void AerialLiDARcloud::setCellCenter( const uint index, const helios::vec3 center ){

  if( index>=getGridCellCount() ){
    cout << "ERROR (setCellCenter): grid cell index out of range.  Requested to set center of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  grid_cells.at(index).center = center;
  
}

helios::vec3 AerialLiDARcloud::getCellGlobalAnchor( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellGlobalAnchor): grid cell index out of range.  Requested anchor of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  return grid_cells.at(index).global_anchor;
  
}

helios::vec3 AerialLiDARcloud::getCellSize( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellCenter): grid cell index out of range.  Requested size of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  return grid_cells.at(index).size;
  
}

float AerialLiDARcloud::getCellRotation( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellRotation): grid cell index out of range.  Requested rotation of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  return grid_cells.at(index).azimuthal_rotation;
  
}

void AerialLiDARcloud::setCellLeafArea( const float area, const uint index ){

  if( index>getGridCellCount() ){
    cout << "ERROR (setCellLeafArea): grid cell index out of range." << endl;
  }

  grid_cells.at(index).leaf_area = area;

}

float AerialLiDARcloud::getCellLeafArea( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellLeafArea): grid cell index out of range. Requested leaf area of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  return grid_cells.at(index).leaf_area;

}

float AerialLiDARcloud::getCellLeafAreaDensity( const uint index ) const{

  if( index>=getGridCellCount() ){
    cout << "ERROR (getCellLeafAreaDensity): grid cell index out of range. Requested leaf area density of cell #" << index << " but there are only " << getGridCellCount() << " cells in the grid." << endl;
    exit(EXIT_FAILURE);
  }

  helios::vec3 gridsize = grid_cells.at(index).size;
  return grid_cells.at(index).leaf_area/(gridsize.x*gridsize.y*gridsize.z);

}