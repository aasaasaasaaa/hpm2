<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>함평군 카드 관리 앱</title>
    <!-- Tailwind CSS CDN -->
    <script src="https://cdn.tailwindcss.com"></script>
    <!-- Google Font: Inter (기본 설정) -->
    <style>
        body { font-family: 'Inter', sans-serif; }
        .card-item {
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        .card-item:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
        }
    </style>
</head>
<body class="bg-gray-50 min-h-screen p-4 md:p-8">
    <div id="app" class="max-w-4xl mx-auto">
        <header class="text-center mb-8">
            <h1 class="text-4xl font-extrabold text-blue-800 tracking-tight">함평군 정보 카드 관리</h1>
            <p class="text-gray-600 mt-2">Firebase Firestore를 이용해 데이터를 저장하고 불러옵니다.</p>
            <div id="statusMessage" class="mt-4 p-2 text-sm font-medium rounded-lg hidden"></div>
        </header>

        <!-- 카드 추가 폼 -->
        <div class="bg-white p-6 rounded-xl shadow-lg mb-8">
            <h2 class="text-2xl font-semibold text-gray-700 mb-4">새 카드 추가</h2>
            <form id="cardForm" class="space-y-4">
                <div>
                    <label for="title" class="block text-sm font-medium text-gray-700">제목 (필수)</label>
                    <input type="text" id="title" required
                           class="mt-1 block w-full rounded-lg border-gray-300 shadow-sm focus:border-blue-500 focus:ring-blue-500 p-3 border">
                </div>
                <div>
                    <label for="content" class="block text-sm font-medium text-gray-700">내용</label>
                    <textarea id="content" rows="4"
                              class="mt-1 block w-full rounded-lg border-gray-300 shadow-sm focus:border-blue-500 focus:ring-blue-500 p-3 border"></textarea>
                </div>
                <div>
                    <label for="category" class="block text-sm font-medium text-gray-700">카테고리</label>
                    <input type="text" id="category"
                           class="mt-1 block w-full rounded-lg border-gray-300 shadow-sm focus:border-blue-500 focus:ring-blue-500 p-3 border">
                </div>
                <button type="submit"
                        class="w-full py-3 px-4 border border-transparent rounded-xl shadow-md text-white font-semibold bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition duration-150 ease-in-out">
                    카드 저장 (Firebase)
                </button>
            </form>
        </div>

        <!-- 카드 목록 -->
        <div class="bg-white p-6 rounded-xl shadow-lg">
            <h2 class="text-2xl font-semibold text-gray-700 mb-4">저장된 카드 목록</h2>
            <div id="cardList" class="grid grid-cols-1 md:grid-cols-2 gap-4">
                <p id="loadingIndicator" class="text-gray-500 col-span-full text-center">데이터를 불러오는 중입니다...</p>
            </div>
            <p id="noCardsMessage" class="text-gray-500 text-center mt-4 hidden">아직 저장된 카드가 없습니다.</p>
        </div>

    </div>

    <script type="module">
        // --- Firebase CDN Imports (GitHub Pages용) ---
        import { initializeApp } from "https://www.gstatic.com/firebasejs/10.13.0/firebase-app.js";
        import { 
            getFirestore, 
            doc, 
            setDoc, 
            getDoc, 
            collection, 
            query, 
            getDocs 
        } from "https://www.gstatic.com/firebasejs/10.13.0/firebase-firestore.js";

        // --- Firebase Configuration (사용자 제공 값) ---
        const firebaseConfig = {
            apiKey: "AIzaSyDS4tth4jcq8CBgNiL2xylwZ49UncZrWCU",
            authDomain: "hampyeongman-5a5e3.firebaseapp.com",
            projectId: "hampyeongman-5a5e3",
            storageBucket: "hampyeongman-5a5e3.firebasestorage.app",
            messagingSenderId: "766021787942",
            appId: "1:766021787942:web:73b911069a7ebca47ee1e7",
            measurementId: "G-J58LPCVNKL"
        };

        // --- Firebase 초기화 및 Firestore 인스턴스 ---
        const app = initializeApp(firebaseConfig);
        const db = getFirestore(app);
        const COLLECTION_NAME = "hampyeong_cards"; // 카드 저장용 컬렉션 이름

        // DOM 요소 참조
        const cardForm = document.getElementById('cardForm');
        const cardList = document.getElementById('cardList');
        const loadingIndicator = document.getElementById('loadingIndicator');
        const noCardsMessage = document.getElementById('noCardsMessage');
        const statusMessage = document.getElementById('statusMessage');

        /**
         * 사용자에게 상태 메시지를 표시합니다.
         * @param {string} message - 표시할 메시지
         * @param {string} type - 'success' 또는 'error'
         */
        function showStatus(message, type) {
            statusMessage.textContent = message;
            statusMessage.classList.remove('hidden', 'bg-green-100', 'text-green-800', 'bg-red-100', 'text-red-800');
            if (type === 'success') {
                statusMessage.classList.add('bg-green-100', 'text-green-800');
            } else if (type === 'error') {
                statusMessage.classList.add('bg-red-100', 'text-red-800');
            }
            setTimeout(() => {
                statusMessage.classList.add('hidden');
            }, 5000);
        }

        /**
         * Firestore에 새 카드 데이터를 저장합니다.
         * @param {object} cardData - 저장할 카드 데이터
         */
        async function saveCardToFirebase(cardData) {
            try {
                // 문서 ID를 현재 시간을 기반으로 생성 (고유성을 위해)
                const docId = Date.now().toString();
                
                await setDoc(doc(db, COLLECTION_NAME, docId), {
                    ...cardData,
                    timestamp: docId // 문서 ID를 타임스탬프로 사용
                });
                showStatus("✅ 카드가 성공적으로 저장되었습니다!", 'success');
                // 저장 후 카드 목록을 다시 불러옵니다.
                await loadCardsFromFirebase(); 

            } catch (error) {
                console.error("Firebase에 카드 저장 중 오류 발생:", error);
                showStatus(`❌ 카드 저장 실패: ${error.message}. Firestore 규칙을 확인하세요!`, 'error');
            }
        }

        /**
         * Firestore에서 모든 카드 데이터를 불러옵니다.
         */
        async function loadCardsFromFirebase() {
            cardList.innerHTML = '';
            loadingIndicator.classList.remove('hidden');
            noCardsMessage.classList.add('hidden');

            try {
                const q = query(collection(db, COLLECTION_NAME));
                const querySnapshot = await getDocs(q);

                const cards = [];
                querySnapshot.forEach((doc) => {
                    // Firestore 문서의 ID와 데이터를 포함합니다.
                    cards.push({ id: doc.id, ...doc.data() });
                });

                // JavaScript 내에서 타임스탬프를 기준으로 정렬 (최신순)
                cards.sort((a, b) => parseInt(b.timestamp) - parseInt(a.timestamp));
                
                loadingIndicator.classList.add('hidden');
                
                if (cards.length === 0) {
                    noCardsMessage.classList.remove('hidden');
                } else {
                    cards.forEach(displayCard);
                }

            } catch (error) {
                console.error("Firebase에서 카드 불러오기 중 오류 발생:", error);
                loadingIndicator.textContent = "데이터 불러오기 실패. 콘솔 오류를 확인하세요.";
                showStatus(`❌ 데이터 로드 실패: ${error.message}`, 'error');
            }
        }

        /**
         * 카드 데이터를 화면에 표시합니다.
         * @param {object} card - 표시할 카드 데이터
         */
        function displayCard(card) {
            const cardElement = document.createElement('div');
            cardElement.className = 'card-item bg-gray-50 p-4 border border-gray-200 rounded-lg shadow-sm';
            
            const titleElement = document.createElement('h3');
            titleElement.className = 'text-lg font-bold text-gray-800 mb-1';
            titleElement.textContent = card.title;

            const contentElement = document.createElement('p');
            contentElement.className = 'text-gray-600 text-sm mb-2';
            contentElement.textContent = card.content || '내용 없음';
            
            const categoryElement = document.createElement('span');
            categoryElement.className = 'inline-block bg-blue-100 text-blue-800 text-xs font-medium px-2.5 py-0.5 rounded-full';
            categoryElement.textContent = card.category || '기타';

            cardElement.appendChild(titleElement);
            cardElement.appendChild(contentElement);
            cardElement.appendChild(categoryElement);
            cardList.appendChild(cardElement);
        }

        // --- 이벤트 리스너 ---
        document.addEventListener('DOMContentLoaded', () => {
            // 폼 제출 이벤트 핸들러
            cardForm.addEventListener('submit', (e) => {
                e.preventDefault();

                const title = document.getElementById('title').value;
                const content = document.getElementById('content').value;
                const category = document.getElementById('category').value;

                if (!title) {
                    showStatus("제목은 필수 입력 사항입니다.", 'error');
                    return;
                }

                const newCard = {
                    title: title,
                    content: content,
                    category: category,
                    createdAt: new Date().toISOString()
                };

                saveCardToFirebase(newCard);

                // 폼 초기화
                cardForm.reset();
            });

            // 페이지 로드 시 카드 데이터 불러오기
            loadCardsFromFirebase();
        });

    </script>
</body>
</html>
