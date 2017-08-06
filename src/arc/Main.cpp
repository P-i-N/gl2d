#define GL3D_IMPLEMENTATION
#include <gl3d/gl3d_win32.h>

using namespace gl3d;

int main()
{
	window_open("Angle measurement", 640, 480);

	float radius = 0.0f;
	float arcSum = 0.0f;
	vec2 baseDirection;
	vec2 prevDirection;

	on_event += [&](event &e)
	{
		if (e.canceled)
			return;

		switch (e.type)
		{
			case event_type::mouse_down:
			{
				if (e.mouse.button == mouse_button::left)
				{
					vec2 center = get_window_center(e.window_id);

					radius = distance(ivec2(e.mouse.x, e.mouse.y), center);
					arcSum = 0.0f;
					baseDirection = normalize(vec2(mouse.x, mouse.y) - center);
					prevDirection = baseDirection;
				}
			}
			break;

			case event_type::mouse_up:
			{
				radius = 0.0f;
			}
			break;

			case event_type::mouse_move:
			{
				if (radius > 0.0f)
				{
					vec2 center = get_window_center(e.window_id);
					vec2 dir = normalize(vec2(mouse.x, mouse.y) - center);
					auto angleDiff = acos(dot(prevDirection, dir));
					
					vec2 perpendicular(-prevDirection.y, prevDirection.x);
					arcSum += sign(dot(perpendicular, dir)) * angleDiff;

					prevDirection = dir;
				}
			}
			break;

			case event_type::paint:
			{
				auto ctx = state.ctx2d;
				vec2 center = get_window_center(state.current_window_id);

				ctx->color(0.2f, 0.2f, 0.2f);
				ctx->line({ center.x, 0 }, { center.x, center.y * 2 });
				ctx->line({ 0, center.y }, { center.x * 2, center.y });

				if (radius > 0.0f)
				{
					// Render filled arc
					{
						if (arcSum >= 0)
							ctx->color(0.0f, 1.0f, 0.0f, 0.25f);
						else
							ctx->color(1.0f, 0.0f, 0.0f, 0.25f);
						
						vec2 dir = normalize(vec2(mouse.x, mouse.y) - center);

						float baseAngle = atan2(baseDirection.x, baseDirection.y);

						for (int i = 0, S = static_cast<int>(abs(degrees(arcSum))) + 1; i < S; ++i)
						{
							float a0 = baseAngle - (i / static_cast<float>(S) * arcSum);
							float a1 = baseAngle - ((i + 1) / static_cast<float>(S) * arcSum);

							vec2 p0(center.x + sin(a0) * radius, center.y + cos(a0) * radius);
							vec2 p1(center.x + sin(a1) * radius, center.y + cos(a1) * radius);

							ctx->triangle(p0, p1, center, true);
						}

						if (arcSum >= 0)
							ctx->color(0.0f, 1.0f, 0.0f);
						else
							ctx->color(1.0f, 0.0f, 0.0f);

						// Render start-end arc lines
						ctx->line(center, center + baseDirection * radius);
						ctx->line(center, center + dir * radius);
					}

					// Render circle
					{
						ctx->color(0.4f, 0.4f, 0.4f);

						for (int i = 0, S = 100; i < S; ++i)
						{
							float a0 = radians(i / static_cast<float>(S) * 360.0f);
							float a1 = radians((i + 1) / static_cast<float>(S) * 360.0f);

							vec2 p0(center.x + sin(a0) * radius, center.y + cos(a0) * radius);
							vec2 p1(center.x + sin(a1) * radius, center.y + cos(a1) * radius);

							ctx->line(p0, p1);
						}
					}

					ctx->color(1.0f, 1.0f, 1.0f);
					ctx->text(5.0f, 5.0f, "Angular sum: %.1f degrees", degrees(arcSum));
					ctx->text(5.0f, 20.0f, "Radius: %.1f pixels", radius);
				}
			}
			break;
		}
	};

	run();
	return 0;
}
