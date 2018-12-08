#ifndef __GL3D_WIN32_H_IMPL__
	#define __GL3D_WIN32_H_IMPL__
#endif

#include "gl3d_window.h"

#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>
#include <hidsdi.h>
#include <Xinput.h>
#include <shellapi.h>

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "xinput.lib")

#define GL3D_WINDOW_CLASS "gl3d_window"

namespace gl3d {

decltype( on_window_event ) on_window_event;

namespace detail {

bool g_should_quit = false;
unsigned g_next_window_id = 0;

uint64_t g_timer_offset = 0;
uint64_t g_timer_frequency = 0;
uint64_t g_last_timer_counter = 0;
size_t g_mouseCaptureCount = 0;

unsigned g_frame_id = 0;
float g_time = 0.0f;
float g_delta = 0.0f;

std::vector<window::ptr> g_windows;
std::map<int, unsigned> g_xinputPortMap;
std::map<void *, unsigned> g_rawInputPortMap;

//---------------------------------------------------------------------------------------------------------------------
struct window_impl
{
	static LRESULT CALLBACK wnd_proc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
};

//---------------------------------------------------------------------------------------------------------------------
struct window_class
{
	WNDCLASS wndclass;

	window_class()
	{
		wndclass = { 0 };
		wndclass.lpfnWndProc = window_impl::wnd_proc;
		wndclass.hInstance = GetModuleHandle( nullptr );
		wndclass.hbrBackground = GetStockBrush( BLACK_BRUSH );
		wndclass.lpszClassName = TEXT( GL3D_WINDOW_CLASS );
		wndclass.style = CS_OWNDC;
		wndclass.hCursor = LoadCursor( nullptr, IDC_ARROW );
		RegisterClass( &wndclass );
	}

	~window_class() { UnregisterClass( TEXT( GL3D_WINDOW_CLASS ), wndclass.hInstance ); }

} g_window_class;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
window::ptr window::create( std::string_view title, uvec2 size, ivec2 pos, unsigned flags )
{
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	RECT adjustedRect;

	if ( size.x <= 0 )
		size.x = GetSystemMetrics( SM_CXSCREEN );

	if ( size.y <= 0 )
		size.y = GetSystemMetrics( SM_CYSCREEN );

	if ( pos.x == INT_MAX )
		adjustedRect.left = ( GetSystemMetrics( SM_CXSCREEN ) - size.x ) / 2;
	else
		adjustedRect.left = pos.x;

	if ( pos.y == INT_MAX )
		adjustedRect.top = ( GetSystemMetrics( SM_CYSCREEN ) - size.y ) / 2;
	else
		adjustedRect.top = pos.y;

	adjustedRect.right = adjustedRect.left + size.x;
	adjustedRect.bottom = adjustedRect.top + size.y;
	AdjustWindowRectEx( &adjustedRect, style & ~WS_OVERLAPPED, FALSE, 0 );

	std::string title_str( title );
	auto handle = CreateWindow( TEXT( GL3D_WINDOW_CLASS ), title_str.c_str(), style,
	                            adjustedRect.left, adjustedRect.top,
	                            adjustedRect.right - adjustedRect.left, adjustedRect.bottom - adjustedRect.top,
	                            nullptr,
	                            nullptr,
	                            GetModuleHandle( nullptr ), nullptr );

	if ( detail::g_windows.empty() )
	{
		RAWINPUTDEVICE rid[2];

		// Gamepads
		rid[0].usUsagePage = 1;
		rid[0].usUsage = 5;
		rid[0].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
		rid[0].hwndTarget = HWND( handle );

		// Joysticks
		rid[1].usUsagePage = 1;
		rid[1].usUsage = 4;
		rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
		rid[1].hwndTarget = HWND( handle );

		RegisterRawInputDevices( rid, 2, sizeof( RAWINPUTDEVICE ) );
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof( PIXELFORMATDESCRIPTOR ), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		24, 8,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	auto hdc = GetDC( handle );
	auto pf = ChoosePixelFormat( hdc, &pfd );
	SetPixelFormat( hdc, pf, &pfd );

	auto context = std::make_shared<detail::context>(
	                   handle,
	                   detail::g_windows.empty()
	                   ? nullptr
	                   : detail::g_windows.front()->context() );

	if ( !context )
		return nullptr;

	auto &result = detail::g_windows.emplace_back( std::make_shared<window>() );

	result->_flags = flags;
	result->_id = detail::g_next_window_id++;
	result->_native_handle = handle;
	result->_context = context;
	result->_qd = std::make_shared<gl3d::quick_draw>();
	result->_title = title;
	result->_pos = pos;
	result->_size = size;

	DragAcceptFiles( handle, TRUE );

	on_window_event( window_event( window_event::type::open, result->_id ) );
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
window::ptr window::from_id( unsigned id )
{
	for ( const auto &w : detail::g_windows )
		if ( w->_id == id )
			return w;

	return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
window::~window()
{
	_qd.reset();
	_context.reset();
	DestroyWindow( HWND( _native_handle ) );
}

//---------------------------------------------------------------------------------------------------------------------
void window::title( std::string_view text )
{
	if ( _title != text )
	{
		_title = text;
		SetWindowTextA( HWND( _native_handle ), _title.c_str() );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void window::adjust( uvec2 size, ivec2 pos )
{
	if ( pos != _pos || size != _size )
	{
		_pos = pos;
		_size = size;

		RECT rect;
		rect.left = _pos.x;
		rect.top = _pos.y;
		rect.right = rect.left + _size.x;
		rect.bottom = rect.top + _size.y;

		DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		AdjustWindowRectEx( &rect, style & ~WS_OVERLAPPED, FALSE, 0 );

		SetWindowPos(
		    HWND( _native_handle ), HWND( _native_handle ),
		    rect.left, rect.top,
		    rect.right - rect.left, rect.bottom - rect.top,
		    SWP_NOZORDER | SWP_NOREPOSITION );
		/* */
	}
}

//---------------------------------------------------------------------------------------------------------------------
void window::close()
{
	if ( !_native_handle )
		return;

	for ( size_t i = 0; i < detail::g_windows.size(); ++i )
	{
		if ( detail::g_windows[i].get() == this )
		{
			on_window_event( window_event( window_event::type::close, _id ) );

			_context.reset();
			DestroyWindow( HWND( _native_handle ) );
			_native_handle = nullptr;

			detail::g_windows.erase( detail::g_windows.begin() + i );
			detail::g_should_quit |= ( _id == 0 ) || detail::g_windows.empty();
			return;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void window::present()
{
	SwapBuffers( GetDC( HWND( _native_handle ) ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//---------------------------------------------------------------------------------------------------------------------
void update_xinput()
{
	for ( int i = 0; i < XUSER_MAX_COUNT; ++i )
	{
		auto iter = g_xinputPortMap.find( i );
		unsigned port = ( iter != g_xinputPortMap.end() ) ? iter->second : UINT_MAX;

		XINPUT_STATE state;
		ZeroMemory( &state, sizeof( XINPUT_STATE ) );
		if ( XInputGetState( i, &state ) == ERROR_SUCCESS )
		{
			if ( port == UINT_MAX )
			{
				port = detail::gamepad_state::allocate_port();
				g_xinputPortMap[i] = port;
				input_event e( input_event::type::gamepad_connect, UINT_MAX );
				e.gamepad.port = port;
				on_input_event( e );
			}
			else
				port = iter->second;

			auto &g = gamepad[port];
			g.native_handle = nullptr; // nullptr means this is XInput controller

			g.change_button_state( gamepad_button::a, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_A ) != 0 );
			g.change_button_state( gamepad_button::b, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_B ) != 0 );
			g.change_button_state( gamepad_button::x, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_X ) != 0 );
			g.change_button_state( gamepad_button::y, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_Y ) != 0 );
			g.change_button_state( gamepad_button::up, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ) != 0 );
			g.change_button_state( gamepad_button::down, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) != 0 );
			g.change_button_state( gamepad_button::left, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) != 0 );
			g.change_button_state( gamepad_button::right, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ) != 0 );
			g.change_button_state( gamepad_button::thumb_left, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ) != 0 );
			g.change_button_state( gamepad_button::thumb_right, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ) != 0 );
			g.change_button_state( gamepad_button::shoulder_left, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) != 0 );
			g.change_button_state( gamepad_button::shoulder_right, ( state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) != 0 );

			g.change_axis_state( gamepad_axis::thumb_left, { state.Gamepad.sThumbLX / 32767.0f, state.Gamepad.sThumbLY / 32767.0f } );
			g.change_axis_state( gamepad_axis::thumb_right, { state.Gamepad.sThumbRX / 32767.0f, state.Gamepad.sThumbRY / 32767.0f } );
			g.change_axis_state( gamepad_axis::triggers, { state.Gamepad.bLeftTrigger / 255.0f, state.Gamepad.bRightTrigger / 255.0f } );
		}
		else if ( port != UINT_MAX )
		{
			input_event e( input_event::type::gamepad_disconnect, UINT_MAX );
			e.gamepad.port = port;
			on_input_event( e );
			detail::gamepad_state::release_port( port );
			g_xinputPortMap.erase( iter );
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void update_raw_input()
{
	UINT numDevices;
	if ( GetRawInputDeviceList( nullptr, &numDevices, sizeof( RAWINPUTDEVICELIST ) ) )
		return;

	thread_local std::vector<RAWINPUTDEVICELIST> tl_deviceList;
	tl_deviceList.resize( numDevices );

	if ( GetRawInputDeviceList( tl_deviceList.data(), &numDevices, sizeof( RAWINPUTDEVICELIST ) ) == UINT_MAX )
		return;

	for ( auto &g : gamepad )
	{
		if ( !g.connected() )
			continue;

		bool found = false;
		for ( auto &device : tl_deviceList )
		{
			if ( device.hDevice == g.native_handle )
			{
				found = true;
				break;
			}
		}

		if ( found )
			continue;

		auto iter = g_rawInputPortMap.find( g.native_handle );
		if ( iter == g_rawInputPortMap.end() )
			continue;

		assert( iter->second == g.port );
		input_event e( input_event::type::gamepad_disconnect, UINT_MAX );
		e.gamepad.port = g.port;
		on_input_event( e );
		detail::gamepad_state::release_port( g.port );
		g_rawInputPortMap.erase( iter );
		g.native_handle = nullptr;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void update()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	li.QuadPart -= g_timer_offset;

	++detail::g_frame_id;
	detail::g_time = static_cast<float>( li.QuadPart / static_cast<double>( g_timer_frequency ) );
	detail::g_delta = static_cast<float>( ( li.QuadPart - g_last_timer_counter ) / static_cast<double>( g_timer_frequency ) );
	g_last_timer_counter = li.QuadPart;

	update_xinput();
	update_raw_input();

	if ( auto w = window::from_id( 0 ); w != nullptr )
	{
		auto ctx = w->context();
		ctx->make_current();
		glViewport( 0, 0, w->size().x, w->size().y );
	}

	on_tick();

	for ( const auto &w : g_windows )
	{
		auto ctx = w->context();
		ctx->make_current();

		glViewport( 0, 0, w->size().x, w->size().y );
		on_window_event( window_event( window_event::type::paint, w->id() ) );

		auto projMatrix = mat4::make_ortho( 0, float( w->size().x ), float( w->size().y ), 0, -1, 1 );

		auto qd = w->quick_draw();
		qd->render( ctx, mat4(), projMatrix );
		qd->reset();

		w->present();
		ctx->reset();
	}
}

//---------------------------------------------------------------------------------------------------------------------
key vk_to_key( WPARAM vk )
{
	static const std::map<WPARAM, key> vkToKeyMap =
	{
		{ VK_RETURN, key::enter }, { VK_TAB, key::tab }, { VK_BACK, key::backspace }, { VK_ESCAPE, key::escape }, { VK_SPACE, key::space },
		{ 0x41, key::a }, { 0x42, key::b }, { 0x43, key::c }, { 0x44, key::d }, { 0x45, key::e }, { 0x46, key::f }, { 0x47, key::g }, { 0x48, key::h },
		{ 0x49, key::i }, { 0x4a, key::j }, { 0x4b, key::k }, { 0x4c, key::l }, { 0x4d, key::m }, { 0x4e, key::n }, { 0x4f, key::o }, { 0x50, key::p },
		{ 0x51, key::q }, { 0x52, key::r }, { 0x53, key::s }, { 0x54, key::t }, { 0x55, key::u }, { 0x56, key::v }, { 0x57, key::w }, { 0x58, key::x },
		{ 0x59, key::y }, { 0x5a, key::z },
		{ 0x30, key::num_0 }, { 0x31, key::num_1 }, { 0x32, key::num_2 }, { 0x33, key::num_3 }, { 0x34, key::num_4 },
		{ 0x35, key::num_5 }, { 0x36, key::num_6 }, { 0x37, key::num_7 }, { 0x38, key::num_8 }, { 0x39, key::num_9 },
		{ VK_NUMPAD0, key::numpad_0 }, { VK_NUMPAD1, key::numpad_1 }, { VK_NUMPAD2, key::numpad_2 }, { VK_NUMPAD3, key::numpad_3 }, { VK_NUMPAD4, key::numpad_4 },
		{ VK_NUMPAD5, key::numpad_5 }, { VK_NUMPAD6, key::numpad_6 }, { VK_NUMPAD7, key::numpad_7 }, { VK_NUMPAD8, key::numpad_8 }, { VK_NUMPAD9, key::numpad_9 },
		{ VK_UP, key::up }, { VK_RIGHT, key::right }, { VK_DOWN, key::down }, { VK_LEFT, key::left },
		{ VK_INSERT, key::insert }, { VK_DELETE, key::del }, { VK_HOME, key::home }, { VK_END, key::end },
		{ VK_PRIOR, key::page_up }, { VK_NEXT, key::page_down },
		{ VK_F1, key::f1 }, { VK_F2, key::f2 }, { VK_F3, key::f3 }, { VK_F4, key::f4 }, { VK_F5, key::f5 }, { VK_F6, key::f6 },
		{ VK_F7, key::f7 }, { VK_F8, key::f8 }, { VK_F9, key::f9 }, { VK_F10, key::f10 }, { VK_F11, key::f11 }, { VK_F12, key::f12 },
		{ VK_CONTROL, key::ctrl }, { VK_MENU, key::alt }, { VK_SHIFT, key::shift }
	};

	auto iter = vkToKeyMap.find( vk );
	if ( iter != vkToKeyMap.end() )
		return iter->second;

	return key::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
mouse_button mbutton_to_mouse_button( UINT msg )
{
	if ( msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP )
		return mouse_button::left;
	if ( msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP )
		return mouse_button::right;
	if ( msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP )
		return mouse_button::middle;

	return mouse_button::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
mouse_button xbutton_to_mouse_button( WPARAM xb )
{
	auto lo = LOWORD( xb );
	if ( lo == 32 ) return mouse_button::back;
	if ( lo == 64 ) return mouse_button::forward;

	return mouse_button::unknown;
}

//---------------------------------------------------------------------------------------------------------------------
void parse_raw_input( RAWINPUT *raw )
{
	thread_local std::vector<uint8_t> tl_preparsedDataBuffer;
	thread_local std::vector<uint8_t> tl_buttonCapsBuffer;
	thread_local std::vector<uint8_t> tl_valueCapsBuffer;
	thread_local std::vector<USAGE> tl_usages;

	UINT bufferSize = 0;
	if ( GetRawInputDeviceInfo( raw->header.hDevice, RIDI_PREPARSEDDATA, nullptr, &bufferSize ) )
		return;

	if ( !bufferSize )
		return;

	unsigned port = UINT_MAX;
	auto iter = g_rawInputPortMap.find( raw->header.hDevice );
	if ( iter == g_rawInputPortMap.end() )
	{
		port = detail::gamepad_state::allocate_port();
		g_rawInputPortMap[raw->header.hDevice] = port;
		input_event e( input_event::type::gamepad_connect, UINT_MAX );
		e.gamepad.port = port;
		on_input_event( e );
	}
	else
		port = iter->second;

	if ( port == UINT_MAX )
		return;

	tl_preparsedDataBuffer.resize( bufferSize );
	GetRawInputDeviceInfo( raw->header.hDevice, RIDI_PREPARSEDDATA, tl_preparsedDataBuffer.data(), &bufferSize );
	auto preparsedData = reinterpret_cast<PHIDP_PREPARSED_DATA>( tl_preparsedDataBuffer.data() );

	HIDP_CAPS caps;
	HidP_GetCaps( preparsedData, &caps );

	tl_buttonCapsBuffer.resize( sizeof( HIDP_BUTTON_CAPS ) * caps.NumberInputButtonCaps );
	auto buttonCaps = reinterpret_cast<PHIDP_BUTTON_CAPS>( tl_buttonCapsBuffer.data() );
	HidP_GetButtonCaps( HidP_Input, buttonCaps, &caps.NumberInputButtonCaps, preparsedData );

	ULONG numPressedButtons = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
	tl_usages.resize( numPressedButtons );
	HidP_GetUsages( HidP_Input,
	                buttonCaps->UsagePage, 0, tl_usages.data(), &numPressedButtons, preparsedData,
	                reinterpret_cast<PCHAR>( raw->data.hid.bRawData ), raw->data.hid.dwSizeHid );

	auto &g = gamepad[port];
	g.native_handle = raw->header.hDevice;

	uint64_t buttonFlags = 0;

	// Resolve buttons
	for ( ULONG i = 0, S = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1; i < S; ++i )
	{
		auto id = tl_usages[i];
		if ( !id )
			continue;

		buttonFlags |= ( 1ull << tl_usages[i] );
	}

	g.change_button_state( gamepad_button::a, ( buttonFlags & ( 1ull << 3 ) ) != 0 );
	g.change_button_state( gamepad_button::b, ( buttonFlags & ( 1ull << 2 ) ) != 0 );
	g.change_button_state( gamepad_button::x, ( buttonFlags & ( 1ull << 4 ) ) != 0 );
	g.change_button_state( gamepad_button::y, ( buttonFlags & ( 1ull << 1 ) ) != 0 );
	g.change_button_state( gamepad_button::shoulder_left, ( buttonFlags & ( 1ull << 7 ) ) != 0 );
	g.change_button_state( gamepad_button::shoulder_right, ( buttonFlags & ( 1ull << 8 ) ) != 0 );
	g.change_button_state( gamepad_button::thumb_left, ( buttonFlags & ( 1ull << 11 ) ) != 0 );
	g.change_button_state( gamepad_button::thumb_right, ( buttonFlags & ( 1ull << 12 ) ) != 0 );

	auto thumbL = g.pos[+gamepad_axis::thumb_left];
	auto thumbR = g.pos[+gamepad_axis::thumb_right];
	auto triggs = g.pos[+gamepad_axis::triggers];

	tl_valueCapsBuffer.resize( sizeof( HIDP_VALUE_CAPS ) * caps.NumberInputValueCaps );
	auto valueCaps = reinterpret_cast<PHIDP_VALUE_CAPS>( tl_valueCapsBuffer.data() );
	HidP_GetValueCaps( HidP_Input, valueCaps, &caps.NumberInputValueCaps, preparsedData );

	// Resolve valus (axis states)
	for ( unsigned i = 0; i < caps.NumberInputValueCaps; ++i )
	{
		ULONG rawValue;
		HidP_GetUsageValue(
		    HidP_Input, valueCaps[i].UsagePage, 0, valueCaps[i].Range.UsageMin, &rawValue, preparsedData,
		    ( PCHAR )raw->data.hid.bRawData, raw->data.hid.dwSizeHid );

		auto value = clamp( ( rawValue - 127 ) / 127.0f, -1.0f, 1.0f );

		auto type = valueCaps[i].Range.UsageMin;
		if ( type == 57 )
		{
			g.change_button_state( gamepad_button::up, rawValue == 0 || rawValue == 1 || rawValue == 7 );
			g.change_button_state( gamepad_button::right, rawValue == 1 || rawValue == 2 || rawValue == 3 );
			g.change_button_state( gamepad_button::down, rawValue == 3 || rawValue == 4 || rawValue == 5 );
			g.change_button_state( gamepad_button::left, rawValue == 5 || rawValue == 6 || rawValue == 7 );
			continue;
		}

		thumbL.x = ( type == 48 ) ? value : thumbL.x;
		thumbL.y = ( type == 49 ) ? value : thumbL.y;
		thumbR.x = ( type == 50 ) ? value : thumbR.x;
		thumbR.y = ( type == 53 ) ? value : thumbR.y;
		triggs.x = ( type == 51 ) ? value : triggs.x;
		triggs.y = ( type == 52 ) ? value : triggs.y;
	}

	g.change_axis_state( gamepad_axis::thumb_left, thumbL );
	g.change_axis_state( gamepad_axis::thumb_right, thumbR );
	g.change_axis_state( gamepad_axis::triggers, triggs );
}

//---------------------------------------------------------------------------------------------------------------------
void process_raw_input( LPARAM lParam )
{
	thread_local std::vector<uint8_t> tl_rawInputBuffer;

	UINT bufferSize;
	GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, nullptr, &bufferSize, sizeof( RAWINPUTHEADER ) );

	tl_rawInputBuffer.resize( bufferSize );
	GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, tl_rawInputBuffer.data(), &bufferSize, sizeof( RAWINPUTHEADER ) );

	parse_raw_input( reinterpret_cast<RAWINPUT *>( tl_rawInputBuffer.data() ) );
}

//---------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK window_impl::wnd_proc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	for ( const auto &w : g_windows )
	{
		if ( HWND( w->native_handle() ) == hWnd )
		{
			switch ( message )
			{
				case WM_INPUT:
					process_raw_input( lParam );
					return 0;

				case WM_MOUSEMOVE:
					mouse.change_position( { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) }, w->id() );
					return 0;

				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
					if ( !( g_mouseCaptureCount++ ) )
						SetCapture( hWnd );

					mouse.change_button_state( mbutton_to_mouse_button( message ), true, w->id() );
					return 0;

				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
					mouse.change_button_state( mbutton_to_mouse_button( message ), false, w->id() );

					if ( !( --g_mouseCaptureCount ) )
						ReleaseCapture();

					return 0;

				case WM_XBUTTONDOWN:
				{
					auto button = xbutton_to_mouse_button( wParam );
					if ( button != mouse_button::unknown )
						mouse.change_button_state( button, true, w->id() );
				}
				return 0;

				case WM_XBUTTONUP:
				{
					auto button = xbutton_to_mouse_button( wParam );
					if ( button != mouse_button::unknown )
						mouse.change_button_state( button, false, w->id() );
				}
				return 0;

				case WM_MOUSEWHEEL:
				{
					input_event e( input_event::type::mouse_wheel, w->id() );
					e.wheel = { 0, GET_WHEEL_DELTA_WPARAM( wParam ) };
					on_input_event( e );
				}
				return 0;

				case WM_MOUSEHWHEEL:
				{
					input_event e( input_event::type::mouse_wheel, w->id() );
					e.wheel = { GET_WHEEL_DELTA_WPARAM( wParam ), 0 };
					on_input_event( e );
				}
				return 0;

				case WM_KEYDOWN:
				{
					bool isKeyPress = ( lParam & ( 1 << 30 ) ) != 0;
					if ( !isKeyPress )
						keyboard.change_key_state( vk_to_key( wParam ), true, w->id() );
				}
				return 0;

				case WM_KEYUP:
					keyboard.change_key_state( vk_to_key( wParam ), false, w->id() );
					return 0;

				case WM_CHAR:
				{
					input_event e( input_event::type::key_press, w->id() );
					e.keyboard.k = key::unknown;
					e.keyboard.ch = static_cast<int>( wParam );
					on_input_event( e );
				}
				return 0;

				case WM_SIZE:
				{
					window_event e( window_event::type::resize, w->id() );
					e.resize = { LOWORD( lParam ), HIWORD( lParam ) };
					on_window_event( e );
					w->_size = e.resize;
				}
				break;

				case WM_SIZING:
				case WM_PAINT:
					update();
					break;

				case WM_CLOSE:
				{
					window::ptr tempRef( w );
					tempRef->close();
				}
				return 0;

				case WM_CAPTURECHANGED:
					//kvp.second->mouse_capture_ref = 0;
					return 0;

				case WM_MOVE:
				{
					window_event e( window_event::type::move, w->id() );
					e.move = { LOWORD( lParam ), HIWORD( lParam ) };
					on_window_event( e );
					w->_pos = e.move;
				}
				return 0;

				case WM_DROPFILES:
				{
					detail::files_t files;
					auto hDrop = HDROP( wParam );

					unsigned index = 0;
					while ( true )
					{
						TCHAR buff[256];
						if ( !DragQueryFile( hDrop, index, buff, 256 ) )
							break;

						files.push_back( buff );
						++index;
					}

					window_event e( window_event::type::drop_files, w->id() );
					e.files = &files;
					on_window_event( e );
				}
				return 0;

				default:
					break;
			}
			break;
		}
	}

	return DefWindowProc( hWnd, message, wParam, lParam );
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
const unsigned &frame_id = detail::g_frame_id;
const float &time = detail::g_time;
const float &delta = detail::g_delta;

//---------------------------------------------------------------------------------------------------------------------
void run()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	detail::g_timer_offset = li.QuadPart;

	QueryPerformanceFrequency( &li );
	detail::g_timer_frequency = li.QuadPart;

	while ( true )
	{
		MSG msg;
		while ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		if ( !detail::g_should_quit )
		{
			detail::update();
			std::this_thread::yield();
		}
		else
			break;
	}
}

} // namespace gl3d
