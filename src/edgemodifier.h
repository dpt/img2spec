class EdgeModifier : public Modifier
{
public:
	float mThreshold;
	float mV;
	bool mR_en, mG_en, mB_en;
	bool mDirectional;
	bool mSeparate;
	float mDirection;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mThreshold);
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
		write(f, mDirectional);
		write(f, mSeparate);
		write(f, mDirection);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mThreshold);
		read(f, mR_en);
		read(f, mG_en);
		read(f, mB_en);
		read(f, mDirectional);
		read(f, mSeparate);
		read(f, mDirection);
	}

	virtual int gettype()
	{
		return MOD_EDGE;
	}


	EdgeModifier()
	{
		mV = 1;
		mThreshold = 1;
		mOnce = 0;
		mR_en = mG_en = mB_en = true;
		mSeparate = false;
		mDirectional = false;
		mDirection = 0;
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

		if (ImGui::CollapsingHeader("Edge Modifier"))
		{
			ret = common();

			if (ImGui::SliderFloat("##Strength", &mV, 0, 1.0f)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) { gDirty = 1; mV = 1; } ImGui::SameLine();
			ImGui::Text("Strength");

			if (ImGui::SliderFloat("##Threshold  ", &mThreshold, 0, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Threshold   ")) { gDirty = 1; mThreshold = 1; } ImGui::SameLine();
			ImGui::Text("Threshold");

			if (ImGui::SliderFloat("##Direction  ", &mDirection, 0, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Direction   ")) { gDirty = 1; mDirection = 0; } ImGui::SameLine();
			ImGui::Text("Direction");

			if (ImGui::Checkbox("Separate color", &mSeparate)) { gDirty = 1; }ImGui::SameLine();
			if (ImGui::Checkbox("Directional", &mDirectional)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; } 
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		float matrix_x[] =
		{
			-1, 0, 1,
			-2, 0, 2,
			-1, 0, 1
		};

		float matrix_y[] =
		{
			-1, -2, -1,
			 0,  0,  0,
			 1,  2,  1
		};

		float *xedge = new float[3 * 256 * 192];
		float *yedge = new float[3 * 256 * 192];

		int i, j;
		for (i = 0; i < 192; i++)
		{
			for (j = 0; j < 256; j++)
			{
				int c;
				for (c = 0; c < 3; c++)
				{
					float ccx = 0;
					float ccy = 0;
					int x, y, m;
					for (y = -1, m = 0; y < 2; y++)
					{
						for (x = -1; x < 2; x++, m++)
						{
							if (x + j > 0 && x + j < 256 && 
								y + i > 0 && y + i < 192)
							{
								ccx += matrix_x[m] * gBitmapProcFloat[((i + y) * 256 + j + x) * 3 + c];
								ccy += matrix_y[m] * gBitmapProcFloat[((i + y) * 256 + j + x) * 3 + c];
							}
						}
					}
					xedge[(i * 256 + j) * 3 + c] = ccx;
					yedge[(i * 256 + j) * 3 + c] = ccy;
				}
			}
		}

		if (!mSeparate)
		{
			for (i = 0; i < 256 * 192; i++)
			{
				float x = (xedge[i * 3 + 0] + xedge[i * 3 + 1] + xedge[i * 3 + 2]) / 3;
				xedge[i * 3 + 0] = x;
				xedge[i * 3 + 1] = x;
				xedge[i * 3 + 2] = x;
				float y = (yedge[i * 3 + 0] + yedge[i * 3 + 1] + yedge[i * 3 + 2]) / 3;
				yedge[i * 3 + 0] = y;
				yedge[i * 3 + 1] = y;
				yedge[i * 3 + 2] = y;
			}

		}

		if (mDirectional)
		{
			float xdir = (float)sin(mDirection * 2 * M_PI);
			float ydir = (float)cos(mDirection * 2 * M_PI);

			for (i = 0; i < 256 * 192; i++)
			{
				if (mB_en && (xedge[i * 3 + 0] * xdir + yedge[i * 3 + 0] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 0] *= 1.0f - mV;
				if (mG_en && (xedge[i * 3 + 1] * xdir + yedge[i * 3 + 1] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 1] *= 1.0f - mV;
				if (mR_en && (xedge[i * 3 + 2] * xdir + yedge[i * 3 + 2] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 2] *= 1.0f - mV;
			}
		}
		else
		{
			for (i = 0; i < 256 * 192; i++)
			{
				if (mB_en && (abs(xedge[i * 3 + 0]) > mThreshold || abs(yedge[i * 3 + 0]) > mThreshold)) gBitmapProcFloat[i * 3 + 0] *= 1.0f - mV;
				if (mG_en && (abs(xedge[i * 3 + 1]) > mThreshold || abs(yedge[i * 3 + 1]) > mThreshold)) gBitmapProcFloat[i * 3 + 1] *= 1.0f - mV;
				if (mR_en && (abs(xedge[i * 3 + 2]) > mThreshold || abs(yedge[i * 3 + 2]) > mThreshold)) gBitmapProcFloat[i * 3 + 2] *= 1.0f - mV;
			}
		}

		delete[] xedge;
		delete[] yedge;
	}

};
