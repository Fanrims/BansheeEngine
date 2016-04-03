//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsRenderTexturePool.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Settings that control the post-process operation. */
	struct PostProcessSettings
	{
		/** 
		 * Determines minimum luminance value in the eye adaptation histogram. In log2 units (-8 = 1/256). In the range
		 * [-16, 0]. 
		 */
		float histogramLog2Min = -8.0f; 

		/** 
		 * Determines maximum luminance value in the eye adaptation histogram. In log2 units (4 = 16). In the range
		 * [0, 16]. 
		 */
		float histogramLog2Max = 4.0f;
	};

	/** Contains per-camera data used by post process effects. */
	struct PostProcessInfo
	{
		PostProcessSettings settings;

		SPtr<PooledRenderTexture> downsampledSceneTex;
		SPtr<PooledRenderTexture> histogramTex;
		SPtr<PooledRenderTexture> eyeAdaptationTex;
	};

	BS_PARAM_BLOCK_BEGIN(DownsampleParams)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvTexSize)
	BS_PARAM_BLOCK_END

	/** Shader that downsamples a texture to half its size. */
	class DownsampleMat : public RendererMaterial<DownsampleMat>
	{
		RMAT_DEF("PPDownsample.bsl");

	public:
		DownsampleMat();

		/** Renders the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTextureCore>& target, PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output will be written. */
		SPtr<RenderTextureCore> getOutput() const { return mOutput; }
	private:
		DownsampleParams mParams;
		MaterialParamVec2Core mInvTexSize;
		MaterialParamTextureCore mInputTexture;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTextureCore> mOutput;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptHistogramParams)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gPixelOffsetAndSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHistogramParams)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gThreadGroupCount)
	BS_PARAM_BLOCK_END

	/** Shader that creates a luminance histogram used for eye adaptation. */
	class EyeAdaptHistogramMat : public RendererMaterial<EyeAdaptHistogramMat>
	{
		RMAT_DEF("PPEyeAdaptHistogram.bsl");

	public:
		EyeAdaptHistogramMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTextureCore>& target, PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output was written. */
		SPtr<RenderTextureCore> getOutput() const { return mOutput; }
	private:
		EyeAdaptHistogramParams mParams;
		MaterialParamTextureCore mSceneColor;
		MaterialParamLoadStoreTextureCore mOutputTex;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTextureCore> mOutput;

		static const INT32 THREAD_GROUP_SIZE_X = 4;
		static const INT32 THREAD_GROUP_SIZE_Y = 4;
		static const INT32 LOOP_COUNT_X = 8;
		static const INT32 LOOP_COUNT_Y = 8;
	};

	/**
	 * Renders post-processing effects for the provided render target.
	 *
	 * @note	Core thread only.
	 */
	class BS_BSRND_EXPORT PostProcessing : public Module<PostProcessing>
	{
	public:
		/** Renders post-processing effects for the provided render target. */
		void postProcess(const SPtr<RenderTextureCore>& target, PostProcessInfo& ppInfo);
		
	private:
		DownsampleMat mDownsample;
		EyeAdaptHistogramMat mEyeAdaptHistogram;
	};

	/** @} */
}