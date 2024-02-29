class ErrorDiffusionDitherModifier : public Modifier
{
public:
	float mV;
	int mModel;
	int mDirection;
	int mOnce;
	float mErrorClamp;

	virtual const char *getname() { return "ErrorDiffusionDither"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mV);
		SERIALIZE(mModel);
		SERIALIZE(mDirection);
		SERIALIZE(mErrorClamp);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mV);
		DESERIALIZE(mModel);
		DESERIALIZE(mDirection);
		DESERIALIZE(mErrorClamp);
#pragma warning(default:4244; default:4800)
	}

	virtual int gettype()
	{
		return MOD_ERRORDIFFUSION;
	}

	ErrorDiffusionDitherModifier()
	{
		mV = 1;
		mModel = 0;
		mOnce = 0;
		mDirection = 0;
		mErrorClamp = 1;
	}

	virtual int ui()
	{
		int ret = 0;
		ImGui::PushID(mUnique);

		if (!mOnce)
		{
			ImGui::OpenNextNode(1);
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Error Diffusion Dither Modifier"))
		{
			ret = common();

			complexsliderfloat("Strength", &mV, 0, 2, 1, 0.001f);
			if (ImGui::Combo("##Model  ", &mModel, "Floyd-Steinberg\0Jarvis-Judice-Ninke\0Stucki\0Burkes\0Sierra3\0Sierra2\0Sierra2-4A\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##model")) { gDirty = 1;  mModel = (mModel + 7 - 1) % 7; } ImGui::SameLine();
			if (ImGui::Button("+##model")) { gDirty = 1;  mModel = (mModel + 7 + 1) % 7; } ImGui::SameLine();
			if (ImGui::Button("Reset##model     ")) { gDirty = 1; mModel = 0; }ImGui::SameLine();
			ImGui::Text("Model");

			if (ImGui::Combo("##Direction  ", &mDirection, "Left-right\0Right-left\0Bidirectional, left-right first\0Bidirectional, right-left first\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##Direction")) { gDirty = 1;  mDirection = (mDirection + 4 - 1) % 4; } ImGui::SameLine();
			if (ImGui::Button("+##Direction")) { gDirty = 1;  mDirection = (mDirection + 4 + 1) % 4; } ImGui::SameLine();
			if (ImGui::Button("Reset##Direction     ")) { gDirty = 1; mDirection = 0; }ImGui::SameLine();
			ImGui::Text("Direction");

			complexsliderfloat("Maximum error", &mErrorClamp, 0, 2, 1, 0.001f);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		float *data = new float[gDevice->mXRes * gDevice->mYRes * 3];
		memcpy(data, gBitmapProcFloat, sizeof(float) * gDevice->mXRes * gDevice->mYRes * 3);
		int i, j;

		float floyd_steinberg[] =
		{
			0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 7.0f / 16.0f, 0.0f / 16.0f,
			0.0f / 16.0f, 3.0f / 16.0f, 5.0f / 16.0f, 1.0f / 16.0f, 0.0f / 16.0f,
			0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f
		};

		float jarvis_judice_ninke[] =
		{
			0.0f / 48.0f, 0.0f / 48.0f, 0.0f / 48.0f, 7.0f / 48.0f, 5.0f / 48.0f,
			3.0f / 48.0f, 5.0f / 48.0f, 7.0f / 48.0f, 5.0f / 48.0f, 3.0f / 48.0f,
			1.0f / 48.0f, 3.0f / 48.0f, 5.0f / 48.0f, 3.0f / 48.0f, 1.0f / 48.0f
		};

		float stucki[] =
		{
			0.0f / 42.0f, 0.0f / 42.0f, 0.0f / 42.0f, 8.0f / 42.0f, 4.0f / 42.0f,
			2.0f / 42.0f, 4.0f / 42.0f, 8.0f / 42.0f, 4.0f / 42.0f, 2.0f / 42.0f,
			1.0f / 42.0f, 2.0f / 42.0f, 4.0f / 42.0f, 2.0f / 42.0f, 1.0f / 42.0f
		};

		float burkes[] =
		{
			0.0f / 32.0f, 0.0f / 32.0f, 0.0f / 32.0f, 8.0f / 32.0f, 4.0f / 32.0f,
			2.0f / 32.0f, 4.0f / 32.0f, 8.0f / 32.0f, 4.0f / 32.0f, 2.0f / 32.0f,
			0.0f / 32.0f, 0.0f / 32.0f, 0.0f / 32.0f, 0.0f / 32.0f, 0.0f / 32.0f
		};

		float sierra3[] =
		{
			0.0f / 32.0f, 0.0f / 32.0f, 0.0f / 32.0f, 5.0f / 32.0f, 3.0f / 32.0f,
			2.0f / 32.0f, 4.0f / 32.0f, 5.0f / 32.0f, 4.0f / 32.0f, 2.0f / 32.0f,
			0.0f / 32.0f, 2.0f / 32.0f, 3.0f / 32.0f, 2.0f / 32.0f, 0.0f / 32.0f
		};

		float sierra2[] =
		{
			0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 4.0f / 16.0f, 3.0f / 16.0f,
			1.0f / 16.0f, 2.0f / 16.0f, 3.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
			0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f, 0.0f / 16.0f
		};

		float sierra2_4a[] =
		{
			0.0f / 4.0f, 0.0f / 4.0f, 0.0f / 4.0f, 2.0f / 4.0f, 0.0f / 4.0f,
			0.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f, 0.0f / 4.0f, 0.0f / 4.0f,
			0.0f / 4.0f, 0.0f / 4.0f, 0.0f / 4.0f, 0.0f / 4.0f, 0.0f / 4.0f
		};

		float *matrix = floyd_steinberg;

		switch (mModel)
		{
		case 0:
			matrix = floyd_steinberg;
			break;
		case 1:
			matrix = jarvis_judice_ninke;
			break;
		case 2:
			matrix = stucki;
			break;
		case 3:
			matrix = burkes;
			break;
		case 4:
			matrix = sierra3;
			break;
		case 5:
			matrix = sierra2;
			break;
		case 6:
			matrix = sierra2_4a;
			break;
		}

		int xpos, ypos;
		int xpos0 = 0, ypos0 = 0;
		int xposi = 1, yposi = 1;
		int dir = 0;
		switch (mDirection)
		{
		case 0:
		case 2:
			xpos0 = 0;
			ypos0 = 0;
			xposi = 1;
			yposi = 1;
			dir = 0;
			break;
		case 1:
		case 3:
			xpos0 = gDevice->mXRes-1;
			ypos0 = 0;
			xposi = -1;
			yposi = 1;
			dir = 1;
			break;
		}

		for (i = 0, ypos = ypos0; i < gDevice->mYRes; i++, ypos += yposi)
		{
			for (j = 0, xpos = xpos0; j < gDevice->mXRes; j++, xpos += xposi)
			{
				int pos = (ypos * gDevice->mXRes + xpos);

				int col = float_to_color(
					data[pos * 3 + 0],
					data[pos * 3 + 1],
					data[pos * 3 + 2]);

				int approx = gDevice->estimate_rgb(col);

				float r = data[pos * 3 + 2] - ((approx >> 0) & 0xff) / 255.0f;
				float g = data[pos * 3 + 1] - ((approx >> 8) & 0xff) / 255.0f;
				float b = data[pos * 3 + 0] - ((approx >> 16) & 0xff) / 255.0f;

				if (abs(r) > mErrorClamp) r = (r > 0) ? mErrorClamp : -mErrorClamp;
				if (abs(g) > mErrorClamp) g = (g > 0) ? mErrorClamp : -mErrorClamp;
				if (abs(b) > mErrorClamp) b = (b > 0) ? mErrorClamp : -mErrorClamp;

				int x, y;
				for (y = 0; y < 3; y++)
				{
					for (x = 0; x < 5; x++)
					{
						if (ypos + y < gDevice->mYRes && x + xpos - 2 >= 0 && x + xpos - 2 < gDevice->mXRes)
						{
							pos = ((ypos + y) * gDevice->mXRes + xpos + x - 2);
							float m;
							if (dir)
							{
								m = matrix[y * 5 + 4 - x];
							}
							else
							{
								m = matrix[y * 5 + x];
							}

							data[pos * 3 + 2] += r * m;
							data[pos * 3 + 1] += g * m;
							data[pos * 3 + 0] += b * m;
						}
					}
				}

			}
			if (mDirection > 1)
			{
				switch (dir)
				{
				case 0:
					xpos0 = gDevice->mXRes-1;
					ypos0 = 0;
					xposi = -1;
					yposi = 1;
					dir = 1;
					break;
				case 1:
					xpos0 = 0;
					ypos0 = 0;
					xposi = 1;
					yposi = 1;
					dir = 0;
					break;
				}
			}
		}

		// Apply (with strength)
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			if (mB_en) gBitmapProcFloat[i * 3 + 0] += (data[i * 3 + 0] - gBitmapProcFloat[i * 3 + 0]) * mV;
			if (mG_en) gBitmapProcFloat[i * 3 + 1] += (data[i * 3 + 1] - gBitmapProcFloat[i * 3 + 1]) * mV;
			if (mR_en) gBitmapProcFloat[i * 3 + 2] += (data[i * 3 + 2] - gBitmapProcFloat[i * 3 + 2]) * mV;
		}
		delete[] data;
	}

};
