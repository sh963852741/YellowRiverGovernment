#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace dnn;

class OpticalFlow
{
public:
	OpticalFlow(String proto, String model, int height, int width)
	{
		net = readNet(proto, model);
		net.setPreferableBackend(DNN_BACKEND_OPENCV);
		img_high = height;
		img_width = width;
	}

	Mat computeFlow(Mat first_img, Mat second_img)
	{
		const Size& size = Size(img_width,img_high);
		Mat inp0 = blobFromImage(first_img, 1.0, size);
		Mat inp1 = blobFromImage(second_img, 1.0, size);
		net.setInput(inp0, "img0");
		net.setInput(inp1, "img1");
		Mat flow = net.forward();
		Mat output = motionToColor(flow);
		return output;
	}

	Mat motionToColor(Mat flow)
	{
		std::vector<int> vector(256);
		
		for (int i = 0; i < 256; ++i)
		{
			vector[i] = i;
		}

		Mat arr(vector);
		Mat colormap;
		applyColorMap(arr, colormap, COLORMAP_HSV);
		
		colormap.squeeze
		
			colormap = colormap.squeeze(1)

			flow = flow.squeeze(0)
			fx, fy = flow[0, ...], flow[1, ...]
			rad = np.sqrt(fx * *2 + fy * *2)
			maxrad = rad.max() if rad.max() != 0 else 1

			ncols = arr.size
			rad = rad[..., np.newaxis] / maxrad
			a = np.arctan2(-fy / maxrad, -fx / maxrad) / np.pi
			fk = (a + 1) / 2.0 * (ncols - 1)
			k0 = fk.astype(np.int)
			k1 = (k0 + 1) % ncols
			f = fk[..., np.newaxis] - k0[..., np.newaxis]

			col0 = colormap[k0] / 255.0
			col1 = colormap[k1] / 255.0
			col = (1 - f) * col0 + f * col1
			col = np.where(rad <= 1, 1 - rad * (1 - col), col * 0.75)
			output = (255.0 * col).astype(np.uint8)
			return output
	}

private:
	int img_high, img_width;
	Net net;
};
