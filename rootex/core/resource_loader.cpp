#include "resource_loader.h"

#include <d3d11.h>

#include "common/common.h"

#include "core/audio/audio_system.h"
#include "core/renderer/index_buffer.h"
#include "core/renderer/vertex_buffer.h"
#include "core/renderer/vertex_data.h"
#include "script/interpreter.h"

#include "vendor/OBJLoader/Source/OBJ_Loader.h"

HashMap<Ptr<ResourceData>, Ptr<ResourceFile>> ResourceLoader::s_ResourcesDataFiles;
HashMap<ResourceFile::Type, Vector<ResourceFile*>> ResourceLoader::s_ResourceFileLibrary;
objl::Loader ResourceLoader::s_ModelLoader;

TextResourceFile* ResourceLoader::CreateTextResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Text)
		{
			return reinterpret_cast<TextResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	FileBuffer& buffer = OS::LoadFileContents(path);
	ResourceData* resData = new ResourceData(path, buffer);
	TextResourceFile* textRes = new TextResourceFile(ResourceFile::Type::Text, resData);

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(textRes);
	s_ResourceFileLibrary[ResourceFile::Type::Text].push_back(textRes);
	return textRes;
}

TextResourceFile* ResourceLoader::CreateNewTextResourceFile(const String& path)
{
	if (!OS::IsExists(path))
	{
		OS::CreateFileName(path);
	}
	return CreateTextResourceFile(path);
}

LuaTextResourceFile* ResourceLoader::CreateLuaTextResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Lua)
		{
			return reinterpret_cast<LuaTextResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	FileBuffer& buffer = OS::LoadFileContents(path);
	ResourceData* resData = new ResourceData(path, buffer);
	LuaTextResourceFile* luaRes = new LuaTextResourceFile(resData);

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(luaRes);
	s_ResourceFileLibrary[ResourceFile::Type::Lua].push_back(luaRes);

	return luaRes;
}

AudioResourceFile* ResourceLoader::CreateAudioResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Wav)
		{
			return reinterpret_cast<AudioResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	const char* audioBuffer;
	int format;
	int size;
	float frequency;
	ALUT_CHECK(audioBuffer = (const char*)alutLoadMemoryFromFile(
	               OS::GetAbsolutePath(path).generic_string().c_str(),
	               &format,
	               &size,
	               &frequency));

	Vector<char> dataArray;
	dataArray.insert(
	    dataArray.begin(),
	    audioBuffer,
	    audioBuffer + size);
	ResourceData* resData = new ResourceData(path, dataArray);

	AudioResourceFile* audioRes = new AudioResourceFile(resData);
	audioRes->m_DecompressedAudioBuffer = audioBuffer;
	audioRes->m_Format = format;
	audioRes->m_AudioDataSize = size;
	audioRes->m_Frequency = frequency;

	switch (audioRes->getFormat())
	{
	case AL_FORMAT_MONO8:
		audioRes->m_Channels = 1;
		audioRes->m_BitDepth = 8;
		break;

	case AL_FORMAT_MONO16:
		audioRes->m_Channels = 1;
		audioRes->m_BitDepth = 16;
		break;

	case AL_FORMAT_STEREO8:
		audioRes->m_Channels = 2;
		audioRes->m_BitDepth = 8;
		break;

	case AL_FORMAT_STEREO16:
		audioRes->m_Channels = 2;
		audioRes->m_BitDepth = 16;
		break;

	default:
		ERR("Unknown channels and bit depth in WAV data");
	}

	audioRes->m_Duration = size * 8 / (audioRes->m_Channels * audioRes->m_BitDepth);
	audioRes->m_Duration /= frequency;

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(audioRes);
	s_ResourceFileLibrary[ResourceFile::Type::Wav].push_back(audioRes);

	return audioRes;
}

VisualModelResourceFile* ResourceLoader::CreateVisualModelResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Obj)
		{
			return reinterpret_cast<VisualModelResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	PANIC(s_ModelLoader.LoadFile(OS::GetAbsolutePath(path).generic_string()) == false, "Model could not be loaded: " + OS::GetAbsolutePath(path).generic_string());

	VertexData vertex;

	Vector<VertexData> vertices;
	vertices.reserve(s_ModelLoader.LoadedVertices.size());

	for (auto& v : s_ModelLoader.LoadedVertices)
	{
		vertex.m_Position.x = v.Position.X;
		vertex.m_Position.y = v.Position.Y;
		vertex.m_Position.z = v.Position.Z;
		vertex.m_Normal.x = v.Normal.X;
		vertex.m_Normal.y = v.Normal.Y;
		vertex.m_Normal.z = v.Normal.Z;
		vertex.m_TextureCoord.x = v.TextureCoordinate.X;
		vertex.m_TextureCoord.y = v.TextureCoordinate.Y;
		vertices.push_back(vertex);
	}

	FileBuffer& buffer = OS::LoadFileContents(path);
	Ptr<VertexBuffer> vertexBuffer(new VertexBuffer(vertices));
	Ptr<IndexBuffer> indexBuffer(new IndexBuffer(s_ModelLoader.LoadedIndices));

	ResourceData* resData = new ResourceData(path, buffer);
	VisualModelResourceFile* visualRes = new VisualModelResourceFile(std::move(vertexBuffer), std::move(indexBuffer), resData);

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(visualRes);
	s_ResourceFileLibrary[ResourceFile::Type::Obj].push_back(visualRes);

	return visualRes;
}

ImageResourceFile* ResourceLoader::CreateImageResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Image)
		{
			return reinterpret_cast<ImageResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	FileBuffer& buffer = OS::LoadFileContents(path);
	ResourceData* resData = new ResourceData(path, buffer);
	ImageResourceFile* imageRes = new ImageResourceFile(resData);

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(imageRes);
	s_ResourceFileLibrary[ResourceFile::Type::Image].push_back(imageRes);

	return imageRes;
}

FontResourceFile* ResourceLoader::CreateFontResourceFile(const String& path)
{
	for (auto& item : s_ResourcesDataFiles)
	{
		if (item.first->getPath() == path && item.second->getType() == ResourceFile::Type::Font)
		{
			return reinterpret_cast<FontResourceFile*>(item.second.get());
		}
	}

	if (OS::IsExists(path) == false)
	{
		ERR("File not found: " + path);
		return nullptr;
	}

	// File not found in cache, load it only once
	FileBuffer& buffer = OS::LoadFileContents(path);
	ResourceData* resData = new ResourceData(path, buffer);
	FontResourceFile* fontRes = new FontResourceFile(resData);

	s_ResourcesDataFiles[Ptr<ResourceData>(resData)] = Ptr<ResourceFile>(fontRes);
	s_ResourceFileLibrary[ResourceFile::Type::Font].push_back(fontRes);

	return fontRes;
}

void ResourceLoader::SaveResourceFile(ResourceFile* resourceFile)
{
	bool saved = OS::SaveFile(resourceFile->getPath(), resourceFile->getData());
	PANIC(saved == false, "Old resource could not be located for saving file: " + resourceFile->getPath().generic_string());
}

void ResourceLoader::ReloadResourceData(const String& path)
{
	FileBuffer& buffer = OS::LoadFileContents(path);

	for (auto&& [resData, resFile] : s_ResourcesDataFiles)
	{
		if (resData->getPath() == path)
		{
			*resData->getRawData() = buffer;
			break;
		}
	}
}

Vector<ResourceFile*>& ResourceLoader::GetFilesOfType(ResourceFile::Type type)
{
	return s_ResourceFileLibrary[type];
}

HashMap<ResourceFile::Type, Vector<ResourceFile*>>& ResourceLoader::GetAllFiles()
{
	return s_ResourceFileLibrary;
}
