#ifndef DIGGING_H
#define DIGGING_H

#include "wombatinclude.h"
#include "globals.h"
#include "tskcpp.h"
#include <liblnk.h>
#include <zip.h>

// Copyright 2013-2020 Pasquale J. Rinaldi, Jr.
// Distrubted under the terms of the GNU General Public License version 2

#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL

void GenerateThumbnails(QString thumbid);
void GenerateHash(QString itemid);
void GenerateVidThumbnails(QString thumbid);
void GenerateArchiveExpansion(QString thumbid);
void GenerateDigging(QString thumbid);
void GeneratePreDigging(QString thumbid);
QString GenerateCategorySignature(const QMimeType mimetype);
QByteArray ReturnFileContent(QString objectid);

#endif // digging.h
