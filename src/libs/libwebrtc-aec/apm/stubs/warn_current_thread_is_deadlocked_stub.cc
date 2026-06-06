namespace webrtc {

#if defined(WEBRTC_ANDROID) && !defined(WEBRTC_CHROMIUM_BUILD)
void WarnThatTheCurrentThreadIsProbablyDeadlocked() {}
#endif

}  // namespace webrtc
