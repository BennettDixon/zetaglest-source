//
//	map.h:
//
//	This file is part of ZetaGlest <https://github.com/ZetaGlest>
//
//	Copyright (C) 2018  The ZetaGlest team
//
//	ZetaGlest is a fork of MegaGlest <https://megaglest.org>
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.

//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>

#ifndef _GLEST_GAME_MAP_H_
#define _GLEST_GAME_MAP_H_

#ifdef WIN32
#include <winsock2.h>
#include <winsock.h>
#endif

#include "vec.h"
#include "math_util.h"
#include "command_type.h"
#include "logger.h"
#include "object.h"
#include "game_constants.h"
#include "selection.h"
#include <cassert>
#include "unit_type.h"
#include "command.h"
#include "checksum.h"
#include "leak_dumper.h"


namespace Glest {
	namespace Game {

		using Shared::Graphics::Vec4f;
		using Shared::Graphics::Quad2i;
		using Shared::Graphics::Rect2i;
		using Shared::Graphics::Vec4f;
		using Shared::Graphics::Vec2f;
		using Shared::Graphics::Vec2i;
		using Shared::Graphics::Texture2D;

		class Tileset;
		class Unit;
		class Resource;
		class TechTree;
		class GameSettings;
		class World;

		// =====================================================
		// 	class Cell
		//
		///	A map cell that holds info about units present on it
		// =====================================================

		class Cell {
		private:
			Unit * units[fieldCount];	//units on this cell
			Unit *unitsWithEmptyCellMap[fieldCount];	//units with an empty cellmap on this cell
			float height;

		private:
			Cell(Cell&);
			void operator=(Cell&);

		public:
			Cell();

			//get
			inline Unit *getUnit(int field) const {
				if (field >= fieldCount) {
					throw megaglest_runtime_error("Invalid field value" + intToStr(field));
				} return units[field];
			}
			inline Unit *getUnitWithEmptyCellMap(int field) const {
				if (field >= fieldCount) {
					throw megaglest_runtime_error("Invalid field value" + intToStr(field));
				} return unitsWithEmptyCellMap[field];
			}
			inline float getHeight() const {
				return truncateDecimal<float>(height, 6);
			}

			inline void setUnit(int field, Unit *unit) {
				if (field >= fieldCount) {
					throw megaglest_runtime_error("Invalid field value" + intToStr(field));
				} units[field] = unit;
			}
			inline void setUnitWithEmptyCellMap(int field, Unit *unit) {
				if (field >= fieldCount) {
					throw megaglest_runtime_error("Invalid field value" + intToStr(field));
				} unitsWithEmptyCellMap[field] = unit;
			}
			inline void setHeight(float height) {
				this->height = truncateDecimal<float>(height, 6);
			}

			inline bool isFree(Field field) const {
				Unit *unit = getUnit(field);
				bool result = (unit == NULL || unit->isPutrefacting());

				if (result == false) {
					//printf("[%s] Line: %d returning false, unit id = %d [%s]\n",__FUNCTION__,__LINE__,getUnit(field)->getId(),getUnit(field)->getType()->getName().c_str());
				}

				return result;
			}

			inline bool isFreeOrMightBeFreeSoon(Vec2i originPos, Vec2i cellPos, Field field) const {
				Unit *unit = getUnit(field);
				bool result = (unit == NULL || unit->isPutrefacting());

				if (result == false) {
					if (originPos.dist(cellPos) > 5 && unit->getType()->isMobile() == true) {
						result = true;
					}

					//printf("[%s] Line: %d returning false, unit id = %d [%s]\n",__FUNCTION__,__LINE__,getUnit(field)->getId(),getUnit(field)->getType()->getName().c_str());
				}

				return result;
			}

			void saveGame(XmlNode *rootNode, int index) const;
			void loadGame(const XmlNode *rootNode, int index, World *world);
		};

		// =====================================================
		// 	class SurfaceCell
		//
		//	A heightmap cell, each surface cell is composed by more than one Cell
		// =====================================================

		class SurfaceCell {
		private:
			//geometry
			Vec3f vertex;
			Vec3f normal;
			Vec3f color;

			//tex coords
			Vec2f fowTexCoord;		//tex coords for TEXTURE1 when multitexturing and fogOfWar
			Vec2f surfTexCoord;		//tex coords for TEXTURE0

			//surface
			int surfaceType;
			const Texture2D *surfaceTexture;

			//object & resource
			Object *object;

			//visibility
			bool visible[GameConstants::maxPlayers + GameConstants::specialFactions];
			bool explored[GameConstants::maxPlayers + GameConstants::specialFactions];

			//cache
			bool nearSubmerged;
			bool cellChangedFromOriginalMapLoad;

		public:
			SurfaceCell();
			~SurfaceCell();

			void end(); //to kill particles
			//get
			inline const Vec3f &getVertex() const {
				return vertex;
			}
			inline float getHeight() const {
				return vertex.y;
			}
			inline const Vec3f &getColor() const {
				return color;
			}
			inline const Vec3f &getNormal() const {
				return normal;
			}
			inline int getSurfaceType() const {
				return surfaceType;
			}
			inline const Texture2D *getSurfaceTexture() const {
				return surfaceTexture;
			}
			inline Object *getObject() const {
				return object;
			}
			inline Resource *getResource() const {
				return object == NULL ? NULL : object->getResource();
			}
			inline const Vec2f &getFowTexCoord() const {
				return fowTexCoord;
			}
			inline const Vec2f &getSurfTexCoord() const {
				return surfTexCoord;
			}
			inline bool getNearSubmerged() const {
				return nearSubmerged;
			}

			inline bool isVisible(int teamIndex) const {
				return visible[teamIndex];
			}
			inline bool isExplored(int teamIndex) const {
				return explored[teamIndex];
			}
			string isVisibleString() const;
			string isExploredString() const;

			//set
			inline void setVertex(const Vec3f &vertex) {
				this->vertex = vertex;
			}
			inline void setHeight(float height, bool cellChangedFromOriginalMapLoadValue = false);
			inline void setNormal(const Vec3f &normal) {
				this->normal = normal;
			}
			inline void setColor(const Vec3f &color) {
				this->color = color;
			}
			inline void setSurfaceType(int surfaceType) {
				this->surfaceType = surfaceType;
			}
			inline void setSurfaceTexture(const Texture2D *st) {
				this->surfaceTexture = st;
			}
			inline void setObject(Object *object) {
				this->object = object;
			}
			inline void setFowTexCoord(const Vec2f &ftc) {
				this->fowTexCoord = ftc;
			}
			inline void setSurfTexCoord(const Vec2f &stc) {
				this->surfTexCoord = stc;
			}
			void setExplored(int teamIndex, bool explored);
			void setVisible(int teamIndex, bool visible);
			inline void setNearSubmerged(bool nearSubmerged) {
				this->nearSubmerged = nearSubmerged;
			}

			//misc
			void deleteResource();
			bool decAmount(int value);
			inline bool isFree() const {
				bool result = (object == NULL || object->getWalkable());

				if (result == false) {
					//printf("[%s] Line: %d returning false\n",__FUNCTION__,__LINE__);
				}
				return result;
			}
			bool getCellChangedFromOriginalMapLoad() const {
				return cellChangedFromOriginalMapLoad;
			}

			void saveGame(XmlNode *rootNode, int index) const;
			void loadGame(const XmlNode *rootNode, int index, World *world);
		};


		// =====================================================
		// 	class Map
		//
		///	Represents the game map (and loads it from a gbm file)
		// =====================================================

		class FastAINodeCache {
		public:
			explicit FastAINodeCache(Unit *unit) {
				this->unit = unit;
			}
			Unit *unit;
			std::map<Vec2i, std::map<Vec2i, bool> > cachedCanMoveSoonList;
		};

		class Map {
		public:
			static const int cellScale;	//number of cells per surfaceCell
			static const int mapScale;	//horizontal scale of surface

		private:
			string title;
			float waterLevel;
			float heightFactor;
			float cliffLevel;
			int cameraHeight;
			int w;
			int h;
			int surfaceW;
			int surfaceH;
			int surfaceSize;

			int hardMaxPlayers; // the max players hard-coded into a map
			int maxPlayers;
			Cell *cells;
			SurfaceCell *surfaceCells;
			Vec2i *startLocations;
			Checksum checksumValue;
			float maxMapHeight;
			string mapFile;

		private:
			Map(Map&);
			void operator=(Map&);

		public:
			Map();
			~Map();
			void end(); //to kill particles
			Checksum * getChecksumValue() {
				return &checksumValue;
			}

			void init(Tileset *tileset);
			Checksum load(const string &path, TechTree *techTree, Tileset *tileset);

			//get
			inline Cell *getCell(int x, int y, bool errorOnInvalid = true) const {
				int arrayIndex = y * w + x;
				if (arrayIndex < 0 || arrayIndex >= getCellArraySize()) {
					if (errorOnInvalid == false) {
						return NULL;
					}
					//abort();
					throw megaglest_runtime_error("arrayIndex >= getCellArraySize(), arrayIndex = " + intToStr(arrayIndex) + " w = " + intToStr(w) + " h = " + intToStr(h));
				} else if (cells == NULL) {
					if (errorOnInvalid == false) {
						return NULL;
					}

					throw megaglest_runtime_error("cells == NULL");
				}

				return &cells[arrayIndex];
			}
			inline Cell *getCell(const Vec2i &pos) const {
				return getCell(pos.x, pos.y);
			}

			inline int getCellArraySize() const {
				return (w * h);
			}
			inline int getSurfaceCellArraySize() const {
				//return (surfaceW * surfaceH);
				return surfaceSize;
			}
			inline SurfaceCell *getSurfaceCell(int sx, int sy) const {
				int arrayIndex = sy * surfaceW + sx;
				if (arrayIndex < 0 || arrayIndex >= getSurfaceCellArraySize()) {
					throw megaglest_runtime_error("arrayIndex >= getSurfaceCellArraySize(), arrayIndex = " + intToStr(arrayIndex) +
						" surfaceW = " + intToStr(surfaceW) + " surfaceH = " + intToStr(surfaceH) +
						" sx: " + intToStr(sx) + " sy: " + intToStr(sy));
				} else if (surfaceCells == NULL) {
					throw megaglest_runtime_error("surfaceCells == NULL");
				}
				return &surfaceCells[arrayIndex];
			}
			inline SurfaceCell *getSurfaceCell(const Vec2i &sPos) const {
				return getSurfaceCell(sPos.x, sPos.y);
			}

			inline int getW() const {
				return w;
			}
			inline int getH() const {
				return h;
			}
			inline int getSurfaceW() const {
				return surfaceW;
			}
			inline int getSurfaceH() const {
				return surfaceH;
			}
			inline int getMaxPlayers() const {
				return maxPlayers;
			}
			inline float getHeightFactor() const {
				return truncateDecimal<float>(heightFactor, 6);
			}
			inline float getWaterLevel() const {
				return truncateDecimal<float>(waterLevel, 6);
			}
			inline float getCliffLevel() const {
				return truncateDecimal<float>(cliffLevel, 6);
			}
			inline int getCameraHeight() const {
				return cameraHeight;
			}
			inline float getMaxMapHeight() const {
				return truncateDecimal<float>(maxMapHeight, 6);
			}
			Vec2i getStartLocation(int locationIndex) const;
			inline bool getSubmerged(const SurfaceCell *sc) const {
				return sc->getHeight() < waterLevel;
			}
			inline bool getSubmerged(const Cell *c) const {
				return c->getHeight() < waterLevel;
			}
			inline bool getDeepSubmerged(const SurfaceCell *sc) const {
				return sc->getHeight() < waterLevel - (1.5f / heightFactor);
			}
			inline bool getDeepSubmerged(const Cell *c) const {
				return c->getHeight() < waterLevel - (1.5f / heightFactor);
			}

			//is
			inline bool isInside(int x, int y) const {
				return x >= 0 && y >= 0 && x < w && y < h;
			}
			inline bool isInside(const Vec2i &pos) const {
				return isInside(pos.x, pos.y);
			}
			inline bool isInsideSurface(int sx, int sy) const {
				return sx >= 0 && sy >= 0 && sx < surfaceW && sy < surfaceH;
			}
			inline bool isInsideSurface(const Vec2i &sPos) const {
				return isInsideSurface(sPos.x, sPos.y);
			}
			bool isResourceNear(int frameIndex, const Vec2i &pos, const ResourceType *rt, Vec2i &resourcePos, int size, Unit *unit = NULL, bool fallbackToPeersHarvestingSameResource = false, Vec2i *resourceClickPos = NULL) const;

			//free cells
			bool isFreeCell(const Vec2i &pos, Field field, bool buildingsOnly = false) const;
			bool isFreeCellOrHasUnit(const Vec2i &pos, Field field, const Unit *unit) const;
			bool isAproxFreeCell(const Vec2i &pos, Field field, int teamIndex) const;
			bool isFreeCells(const Vec2i &pos, int size, Field field, bool buildingsOnly = false) const;
			bool isFreeCellsOrHasUnit(const Vec2i &pos, int size, Field field, const Unit *unit) const;
			bool isAproxFreeCells(const Vec2i &pos, int size, Field field, int teamIndex) const;
			bool canMorph(const Vec2i &pos, const Unit *currentUnit, const UnitType *targetUnitType) const;
			//bool canOccupy(const Vec2i &pos, Field field, const UnitType *ut, CardinalDir facing);

			//unit placement
			bool aproxCanMove(const Unit *unit, const Vec2i &pos1, const Vec2i &pos2, std::map<Vec2i, std::map<Vec2i, std::map<int, std::map<int, std::map<Field, bool> > > > > *lookupCache = NULL) const;
			bool canMove(const Unit *unit, const Vec2i &pos1, const Vec2i &pos2, std::map<Vec2i, std::map<Vec2i, std::map<int, std::map<Field, bool> > > > *lookupCache = NULL) const;
			void putUnitCells(Unit *unit, const Vec2i &pos, bool ignoreSkill = false, bool threaded = false);
			void clearUnitCells(Unit *unit, const Vec2i &pos, bool ignoreSkill = false);

			Vec2i computeRefPos(const Selection *selection) const;
			Vec2i computeDestPos(const Vec2i &refUnitPos, const Vec2i &unitPos,
				const Vec2i &commandPos) const;
			const Unit * findClosestUnitToPos(const Selection *selection, Vec2i originalBuildPos,
				const UnitType *ut) const;
			bool isInUnitTypeCells(const UnitType *ut, const Vec2i &pos, const Vec2i &testPos) const;
			bool isNextToUnitTypeCells(const UnitType *ut, const Vec2i &pos, const Vec2i &testPos) const;
			Vec2i findBestBuildApproach(const Unit *unit, Vec2i originalBuildPos, const UnitType *ut) const;
			//std::pair<float,Vec2i> getUnitDistanceToPos(const Unit *unit,Vec2i pos,const UnitType *ut);

			//misc
			bool isNextTo(const Vec2i &pos, const Unit *unit) const;
			bool isNextTo(const Vec2i &pos, const Vec2i &nextToPos) const;
			bool isNextTo(const Unit *unit1, const Unit *unit2) const;
			void clampPos(Vec2i &pos) const;

			void prepareTerrain(const Unit *unit);
			void flatternTerrain(const Unit *unit);
			void computeNormals();
			void computeInterpolatedHeights();

			//static
			inline static Vec2i toSurfCoords(const Vec2i &unitPos) {
				return unitPos / cellScale;
			}
			inline static Vec2i toUnitCoords(const Vec2i &surfPos) {
				return surfPos * cellScale;
			}

			inline bool isFreeCellOrMightBeFreeSoon(Vec2i originPos, const Vec2i &pos, Field field) const {
				return
					isInside(pos) &&
					isInsideSurface(toSurfCoords(pos)) &&
					getCell(pos)->isFreeOrMightBeFreeSoon(originPos, pos, field) &&
					(field == fAir || getSurfaceCell(toSurfCoords(pos))->isFree()) &&
					(field != fLand || getDeepSubmerged(getCell(pos)) == false);
			}

			inline bool isAproxFreeCellOrMightBeFreeSoon(Vec2i originPos, const Vec2i &pos, Field field, int teamIndex) const {
				if (isInside(pos) && isInsideSurface(toSurfCoords(pos))) {
					const SurfaceCell *sc = getSurfaceCell(toSurfCoords(pos));

					if (sc->isVisible(teamIndex)) {
						return isFreeCellOrMightBeFreeSoon(originPos, pos, field);
					} else if (sc->isExplored(teamIndex)) {
						return field == fLand ? sc->isFree() && !getDeepSubmerged(getCell(pos)) : true;
					} else {
						return true;
					}
				}

				//printf("[%s] Line: %d returning false\n",__FUNCTION__,__LINE__);
				return false;
			}

			//checks if a unit can move from between 2 cells using only visible cells (for pathfinding)
			inline bool aproxCanMoveSoon(Unit *unit, const Vec2i &pos1, const Vec2i &pos2) const {
				if (isInside(pos1) == false || isInsideSurface(toSurfCoords(pos1)) == false ||
					isInside(pos2) == false || isInsideSurface(toSurfCoords(pos2)) == false) {

					//printf("[%s] Line: %d returning false\n",__FUNCTION__,__LINE__);
					if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
						SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
						char szBuf[8096] = "";
						snprintf(szBuf, 8096, "In aproxCanMoveSoon() return false");
						if (Thread::isCurrentThreadMainThread() == false) {
							unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
						} else {
							unit->logSynchData(__FILE__, __LINE__, szBuf);
						}
					}

					return false;
				}

				if (unit == NULL) {
					throw megaglest_runtime_error("unit == NULL");
				}

				int size = unit->getType()->getSize();
				int teamIndex = unit->getTeam();
				Field field = unit->getCurrField();

				//single cell units
				if (size == 1) {
					bool tryPosResult = isAproxFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), pos2, field, teamIndex);

					if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
						SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
						string extraInfo = (string("tryPosResult = ") + (tryPosResult ? string("true") : string("false")));
						const SurfaceCell *sc = getSurfaceCell(toSurfCoords(pos2));
						if (sc->isVisible(teamIndex)) {
							bool testCond = isFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), pos2, field);
							extraInfo += (string("isFreeCellOrMightBeFreeSoon = ") + (testCond ? string("true") : string("false")));
						} else if (sc->isExplored(teamIndex)) {
							bool testCond = field == fLand ? sc->isFree() && !getDeepSubmerged(getCell(pos2)) : true;
							extraInfo += (string("field==fLand = ") + (testCond ? string("true") : string("false")));
						}

						char szBuf[8096] = "";
						snprintf(szBuf, 8096, "In aproxCanMoveSoon() pos2 = %s extraInfo = %s %s %s", pos2.getString().c_str(), extraInfo.c_str(), sc->isVisibleString().c_str(), sc->isExploredString().c_str());
						if (Thread::isCurrentThreadMainThread() == false) {
							unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
						} else {
							unit->logSynchData(__FILE__, __LINE__, szBuf);
						}
					}

					if (tryPosResult == false) {
						return false;
					}
					if (pos1.x != pos2.x && pos1.y != pos2.y) {
						Vec2i tryPos = Vec2i(pos1.x, pos2.y);
						bool tryPosResult = isAproxFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), tryPos, field, teamIndex);

						if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
							SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
							string extraInfo = (string("tryPosResult = ") + (tryPosResult ? string("true") : string("false")));
							const SurfaceCell *sc = getSurfaceCell(toSurfCoords(tryPos));
							if (sc->isVisible(teamIndex)) {
								bool testCond = isFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), tryPos, field);
								extraInfo += (string("isFreeCellOrMightBeFreeSoon = ") + (testCond ? string("true") : string("false")));
							} else if (sc->isExplored(teamIndex)) {
								bool testCond = field == fLand ? sc->isFree() && !getDeepSubmerged(getCell(tryPos)) : true;
								extraInfo += (string("field==fLand = ") + (testCond ? string("true") : string("false")));
							}

							char szBuf[8096] = "";
							snprintf(szBuf, 8096, "In aproxCanMoveSoon() extraInfo = %s", extraInfo.c_str());
							if (Thread::isCurrentThreadMainThread() == false) {
								unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
							} else {
								unit->logSynchData(__FILE__, __LINE__, szBuf);
							}
						}

						if (tryPosResult == false) {
							return false;
						}

						tryPos = Vec2i(pos2.x, pos1.y);
						tryPosResult = isAproxFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), tryPos, field, teamIndex);

						if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
							SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
							string extraInfo = (string("tryPosResult = ") + (tryPosResult ? string("true") : string("false")));
							const SurfaceCell *sc = getSurfaceCell(toSurfCoords(tryPos));
							if (sc->isVisible(teamIndex)) {
								bool testCond = isFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), tryPos, field);
								extraInfo += (string("isFreeCellOrMightBeFreeSoon = ") + (testCond ? string("true") : string("false")));
							} else if (sc->isExplored(teamIndex)) {
								bool testCond = field == fLand ? sc->isFree() && !getDeepSubmerged(getCell(tryPos)) : true;
								extraInfo += (string("field==fLand = ") + (testCond ? string("true") : string("false")));
							}

							char szBuf[8096] = "";
							snprintf(szBuf, 8096, "In aproxCanMoveSoon() extraInfo = %s", extraInfo.c_str());
							if (Thread::isCurrentThreadMainThread() == false) {
								unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
							} else {
								unit->logSynchData(__FILE__, __LINE__, szBuf);
							}
						}

						if (tryPosResult == false) {

							return false;
						}
					}

					bool isBadHarvestPos = false;
					Command *command = unit->getCurrCommand();
					if (command != NULL) {
						const HarvestCommandType *hct = dynamic_cast<const HarvestCommandType*>(command->getCommandType());
						if (hct != NULL && unit->isBadHarvestPos(pos2) == true) {
							isBadHarvestPos = true;
						}
					}

					if (unit == NULL || isBadHarvestPos == true) {
						if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
							SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
							char szBuf[8096] = "";
							snprintf(szBuf, 8096, "In aproxCanMoveSoon() return false");
							if (Thread::isCurrentThreadMainThread() == false) {
								unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
							} else {
								unit->logSynchData(__FILE__, __LINE__, szBuf);
							}
						}

						return false;
					}

					return true;
				}
				//multi cell units
				else {
					for (int i = pos2.x; i < pos2.x + size; ++i) {
						for (int j = pos2.y; j < pos2.y + size; ++j) {

							Vec2i cellPos = Vec2i(i, j);
							if (isInside(cellPos) && isInsideSurface(toSurfCoords(cellPos))) {
								if (getCell(cellPos)->getUnit(unit->getCurrField()) != unit) {
									if (isAproxFreeCellOrMightBeFreeSoon(unit->getPosNotThreadSafe(), cellPos, field, teamIndex) == false) {
										if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
											SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
											char szBuf[8096] = "";
											snprintf(szBuf, 8096, "In aproxCanMoveSoon() return false");
											if (Thread::isCurrentThreadMainThread() == false) {
												unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
											} else {
												unit->logSynchData(__FILE__, __LINE__, szBuf);
											}
										}

										return false;
									}
								}
							} else {
								if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
									SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
									char szBuf[8096] = "";
									snprintf(szBuf, 8096, "In aproxCanMoveSoon() return false");
									if (Thread::isCurrentThreadMainThread() == false) {
										unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
									} else {
										unit->logSynchData(__FILE__, __LINE__, szBuf);
									}
								}

								return false;
							}
						}
					}

					bool isBadHarvestPos = false;
					Command *command = unit->getCurrCommand();
					if (command != NULL) {
						const HarvestCommandType *hct = dynamic_cast<const HarvestCommandType*>(command->getCommandType());
						if (hct != NULL && unit->isBadHarvestPos(pos2) == true) {
							isBadHarvestPos = true;
						}
					}

					if (isBadHarvestPos == true) {
						if (SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynch).enabled == true &&
							SystemFlags::getSystemSettingType(SystemFlags::debugWorldSynchMax).enabled == true) {
							char szBuf[8096] = "";
							snprintf(szBuf, 8096, "In aproxCanMoveSoon() return false");
							if (Thread::isCurrentThreadMainThread() == false) {
								unit->logSynchDataThreaded(__FILE__, __LINE__, szBuf);
							} else {
								unit->logSynchData(__FILE__, __LINE__, szBuf);
							}
						}

						return false;
					}

				}
				return true;
			}

			string getMapFile() const {
				return mapFile;
			}

			void saveGame(XmlNode *rootNode) const;
			void loadGame(const XmlNode *rootNode, World *world);

		private:
			//compute
			void smoothSurface(Tileset *tileset);
			void computeNearSubmerged();
			void computeCellColors();
			void putUnitCellsPrivate(Unit *unit, const Vec2i &pos, const UnitType *ut, bool isMorph, bool threaded);
		};


		// ===============================
		// 	class PosCircularIterator
		// ===============================

		class PosCircularIterator {
		private:
			Vec2i center;
			int radius;
			const Map *map;
			Vec2i pos;

		public:
			PosCircularIterator(const Map *map, const Vec2i &center, int radius);
			bool next();
			const Vec2i &getPos();
		};

		// ===============================
		// 	class PosQuadIterator
		// ===============================

		class PosQuadIterator {
		private:
			Quad2i quad;
			Rect2i boundingRect;
			Vec2i pos;
			int step;
			const Map *map;

		public:
			PosQuadIterator(const Map *map, const Quad2i &quad, int step = 1);
			bool next();
			//void skipX();
			const Vec2i &getPos();
		};

	}
} //end namespace

#endif
