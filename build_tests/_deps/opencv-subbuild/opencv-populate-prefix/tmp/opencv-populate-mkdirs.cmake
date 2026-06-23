# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-src")
  file(MAKE_DIRECTORY "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-build"
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix"
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/tmp"
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp"
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/src"
  "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/deril/OneDrive/Рабочий стол/проект тестирование/Image_Redactor/build_tests/_deps/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
