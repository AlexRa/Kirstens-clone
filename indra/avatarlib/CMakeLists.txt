# -*- cmake -*-

project(avatarlib)

include(00-Common)
include(LLCommon)



include_directories(
    ${LLCOMMON_INCLUDE_DIRS}
           
    )

set(avatarlib_SOURCE_FILES
    
Avatar.cpp
AvatarAttachment.cpp
AvatarAttachmentSerializer.cpp
AvatarSerializer.cpp
ClientAvatar.cpp
Cloth.cpp
ClothManager.cpp
ClothSerializer.cpp
ClothThread.cpp
CollisionCapsule.cpp
CollisionEllipsoid.cpp
CollisionShape.cpp
DynamicAnimation.cpp
DynamicAnimationManager.cpp
DynamicAnimationSerializer.cpp
DynamicLines.cpp
DynamicRenderable.cpp
Limb.cpp
myvec.cpp
OgreAvatar.cpp
Platform.cpp
PlatformWin.cpp
RexException.cpp
srs.cpp
tinystr.cpp
tinyxml.cpp
tinyxmlerror.cpp
tinyxmlparser.cpp
trig.cpp
XmlDocument.cpp
XmlFile.cpp
XmlManager.cpp

    
    )

set(avatarlib_HEADER_FILES
    CMakeLists.txt

    Avatar.h
AvatarAttachment.h
AvatarAttachmentSerializer.h
AvatarSerializer.h
ClientAvatar.h
Cloth.h
ClothManager.h
ClothSerializer.h
ClothThread.h
CollisionCapsule.h
CollisionEllipsoid.h
CollisionShape.h
DynamicAnimation.h
DynamicAnimationManager.h
DynamicAnimationSerializer.h
DynamicLines.h
DynamicRenderable.h
Limb.h
myvec.h
OgreAvatar.h
Platform.h
PlatformWin.h
RexException.h
srs.h
tinystr.h
tinyxml.h
trig.h
XmlDocument.h
XmlFile.h
XmlManager.h
    
    )

set_source_files_properties(${avatarlib_HEADER_FILES}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND avatarlib_SOURCE_FILES ${avatarlib_HEADER_FILES})

add_library (avatarlib ${avatarlib_SOURCE_FILES})
