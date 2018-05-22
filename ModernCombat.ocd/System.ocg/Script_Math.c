/**
	Some math functions

	@author Marky
 */

global func InterpolateRGBa(int progress, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, interpolation)
{
	interpolation = interpolation ?? Global.InterpolateLinear;
	
	var y0_split = SplitRGBaValue(y0);
	var y1_split = SplitRGBaValue(y1);
	var y2_split = SplitRGBaValue(y2);
	var y3_split = SplitRGBaValue(y3);

	var r = Call(interpolation, progress, x0, y0_split.R, x1, y1_split.R, x2, y2_split.R, x3, y3_split.R);
	var g = Call(interpolation, progress, x0, y0_split.G, x1, y1_split.G, x2, y2_split.G, x3, y3_split.G);
	var b = Call(interpolation, progress, x0, y0_split.B, x1, y1_split.B, x2, y2_split.B, x3, y3_split.B);
	var a = Call(interpolation, progress, x0, y0_split.Alpha, x1, y1_split.Alpha, x2, y2_split.Alpha, x3, y3_split.Alpha);

	return RGBa(r, g, b, a);
}


global func InterpolateLinear(int progress, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
	progress = BoundBy(progress, Min(Min(x0, x1), Min(x2, x3)), Max(Max(x0, x1), Max(x2, x3)));

	if (progress > x1 && x2 || x3)
	{
		if (x2 <= progress && progress <= x3)
		{
			return InterpolateLinear(progress, x2, y2, x3, y3);
		}
		else if (x1 <= progress && progress <= x2)
		{
			return InterpolateLinear(progress, x1, y1, x2, y2);
		}
	}
	else
	{
		var interval = x1 - x0;
		var factor = BoundBy(progress - x0, 0, interval);
		
		return (factor * y1 + (interval - factor) * y0) / Max(1, interval);
	}
}