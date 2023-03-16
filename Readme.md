# Summary

This is a sample UnrealEngine project using a libzip wrapped plugin.

# Support Platform

Win64 Only

# DevelopmentEnvironment

* Windows11
* UnrealEngine5.1.1
* VisualStudio2022

# Dependency

The following is used in the libzip build.

* https://github.com/kiyolee/zlib-win-build.git
* https://github.com/kiyolee/libzip-win-build.git

# Usage

## Sample Project

The following screen appears in Editor Play, where you can try archive and unarchive by specifying the paths.

![Sample](https://user-images.githubusercontent.com/852982/225472474-d53f3eee-a8e8-4c10-8ed0-253e5c8cd622.png)

## Blueprint

Archive
![Archive](https://user-images.githubusercontent.com/852982/225469752-02f0901b-debe-42eb-889e-7589f025b3aa.png)

Unarchive
![Unarchive](https://user-images.githubusercontent.com/852982/225469709-2aac86f5-6bc1-4b5a-ad80-c480b3250faa.png)

## C++

You can see code in the [Automation Spec](https://github.com/mechamogera/UnrealEngineLibzipPlugin/blob/master/Plugins/LibzipArchiver/Source/LibzipArchiver/Spec/Archive.spec.cpp)