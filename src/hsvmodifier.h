
class HSVModifier : public Modifier
{
public:
	float mH, mS, mV;
	int mOnce;

	virtual const char *getname() { return "HSV"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mH);
		SERIALIZE(mS);
		SERIALIZE(mV);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mH);
		DESERIALIZE(mS);
		DESERIALIZE(mV);
#pragma warning(default:4244; default:4800)
	}

	virtual int gettype()
	{
		return MOD_HSV;
	}


	HSVModifier()
	{
		mH = 0;
		mS = 0;
		mV = 0;
		mOnce = 0;
	}

	static void rgb2hsv(float r, float g, float b, float &h, float &s, float &v)
	{
		float      min, max, delta;

		min = r < g ? r : g;
		min = min  < b ? min : b;

		max = r > g ? r : g;
		max = max > b ? max : b;

		v = max;                                // v
		delta = max - min;
		if (delta < 0.00001)
		{
			s = 0;
			h = 0; // undefined, maybe nan?
			return;
		}
		if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
			s = (delta / max);                  // s
		}
		else {
			// if max is 0, then r = g = b = 0              
			// s = 0, v is undefined
			s = 0.0;
			h = 0.0;                            // its now undefined
			return;
		}
		if (r >= max)                           // > is bogus, just keeps compilor happy
			h = (g - b) / delta;        // between yellow & magenta
		else
			if (g >= max)
				h = 2.0f + (b - r) / delta;  // between cyan & yellow
			else
				h = 4.0f + (r - g) / delta;  // between magenta & cyan

		h *= 60.0f;                              // degrees

		if (h < 0.0f)
			h += 360.0f;
	}


	static void hsv2rgb(float h, float s, float v, float &r, float &g, float &b)
	{
		float      hh, p, q, t, ff;
		long        i;

		if (s <= 0.0f) {       // < is bogus, just shuts up warnings
			r = v;
			g = v;
			b = v;
			return;
		}
		hh = h;
		if (hh >= 360.0f) hh = 0.0f;
		hh /= 60.0f;
		i = (long)floor(hh);
		ff = hh - i;
		p = v * (1.0f - s);
		q = v * (1.0f - (s * ff));
		t = v * (1.0f - (s * (1.0f - ff)));

		switch (i) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
		default:
			r = v;
			g = p;
			b = q;
			break;
		}
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

		if (ImGui::CollapsingHeader("HSV Modifier"))
		{
			ret = common();

			complexsliderfloat("Hue (color)", &mH, -360, 360, 0, 1);
			complexsliderfloat("Saturation (richness)", &mS, -2, 2, 0, 0.001f);
			complexsliderfloat("Value (brightness)", &mV, -2, 2, 0, 0.001f);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{

		int i;
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			float h, s, v;
			rgb2hsv(gBitmapProcFloat[i * 3 + 0], gBitmapProcFloat[i * 3 + 1], gBitmapProcFloat[i * 3 + 2], h, s, v);

			h += mH;
			s += mS;
			v += mV;

			if (h < 0) h += 360;
			if (h > 360) h -= 360;

			float r, g, b;
			hsv2rgb(h, s, v, b, g, r);
			if (mB_en) gBitmapProcFloat[i * 3 + 0] = b; 
			if (mG_en) gBitmapProcFloat[i * 3 + 1] = g;
			if (mR_en) gBitmapProcFloat[i * 3 + 2] = r;
		}
	}
};
