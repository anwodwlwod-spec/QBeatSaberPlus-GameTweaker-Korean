# 한국어 패치 메모

대상: QBeatSaberPlus-GameTweaker / Quest Beat Saber 1.40.8 계열

수정 범위
- 설정 화면 제목과 탭
- GameTweaker 설정 항목 전체
- 정보/제작진 화면
- 초기화 확인창과 안내창
- 모듈 이름 및 모듈 선택 화면 설명
- 플레이어 설정에 추가되는 "조명 밝기 재정의" 문구

툴팁/호버 설명
- 이 저장소에는 각 설정별 HoverHint/Tooltip 문구가 별도로 정의되어 있지 않습니다.
- 모듈 선택 화면에서 커서를 올렸을 때 표시되는 설명은 shared/GameTweaker.hpp의 Description()을 한국어로 바꿨습니다.
- Beat Saber 본체가 제공하는 기본 UI 설명은 이 모드 소스만으로 변경되지 않을 수 있습니다.

주의
- Quest에서 사용하는 폰트에 한글 글리프가 없으면 글자가 네모(□)로 보일 수 있습니다. 실제 빌드 후 확인이 필요합니다.
