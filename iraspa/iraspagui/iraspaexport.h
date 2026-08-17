#pragma once

#include <QUrl>
#include <memory>

#include "moviemaker.h"
#include "projectstructure.h"
#include "projecttreenode.h"
#include "rkrendererbackend.h"

void runOffscreenPictureExport(std::shared_ptr<ProjectTreeNode> nodeCopy, QUrl fileURL, int width, int height,
                               RKRendererBackend backend);
void runOffscreenMovieExport(std::shared_ptr<ProjectTreeNode> nodeCopy, QUrl fileURL, int width, int height,
                             MovieWriter::Format format, ProjectStructure::MovieType movieType,
                             RKRendererBackend backend);
