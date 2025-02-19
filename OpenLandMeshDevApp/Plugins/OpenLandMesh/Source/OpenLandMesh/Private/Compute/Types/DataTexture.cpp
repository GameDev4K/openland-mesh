﻿// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Compute/Types/DataTexture.h"

FDataTexture::FDataTexture(int Width)
{
	TextureWidth = Width;

	// Assign the Texture
	Texture = UTexture2D::CreateTransient(TextureWidth, TextureWidth);
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
	Texture->CompressionSettings = TC_VectorDisplacementmap;
	Texture->SRGB = 0;
	Texture->AddToRoot();
	Texture->Filter = TF_Nearest;
	Texture->UpdateResource();

	// Set the Region
	WholeTextureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);

	// Allocate Data
	const int32 BufferSize = TextureWidth * TextureWidth * 4;
	SourceData = new uint8[BufferSize];

	Reset();
}

FDataTexture::~FDataTexture()
{
	if (SourceData != nullptr)
	{
		delete[] SourceData;
		SourceData = nullptr;
	}

	if (Texture != nullptr)
	{
		if (Texture->IsValidLowLevel())
			Texture->ReleaseResource();
		Texture = nullptr;
	}
}

void FDataTexture::SetPixelValue(int32 Index, uint8 R, uint8 G, uint8 B, uint8 A)
{
	uint8* pointer = SourceData + (Index * 4);
	*pointer = B; //b
	*(pointer + 1) = G; //g
	*(pointer + 2) = R; //r
	*(pointer + 3) = A; //a
}

void FDataTexture::SetFloatValue(int32 Index, float Value)
{
	float* ValuePointer = &Value;
	uint8* ValueBytes = reinterpret_cast<uint8*>(ValuePointer);
	SetPixelValue(Index, ValueBytes[0], ValueBytes[1], ValueBytes[2], ValueBytes[3]);
}

void FDataTexture::Reset()
{
	for (int32 Index = 0; Index < TextureWidth * TextureWidth; Index++)
		SetPixelValue(Index, 0, 0, 0, 0);

	UpdateTexture();
}

void FDataTexture::UpdateTexture()
{
	const int BytesPerPixel = 4;
	const int32 BytesPerRow = TextureWidth * BytesPerPixel;
	Texture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), &WholeTextureRegion,
	                              static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), SourceData);
}
