# MarioGame

$ ==================================== $
$ cập nhật việc làm của Phạm Trung Duy $
4 ==================================== $
## Cách xắp sếp files

### Lý do không sử dụng 2 folder src và include
- Vì khi số lượng file .cpp và .h nhiều thì nhìn rất rối và các file xắp theo thứ tự chữ cái thì nó không mang nhiều ý nghĩa.

### Cách xắp xếp mới
- Mỗi folder sẽ chứa  name.cpp và name.h vào cùng 1 folder. Mỗi folder sẽ được sắp xếp theo quan hệ cha con hay chứ năng của các file có trong folder.
- Mục đích: tiện quản lí và nhanh tìm ra file mình cần tìm.

## IGameState
- đây là abstract class mô tả một layer của chương trình. 
- Nó có 4 thao tác chính là <Initialize>: khởi tạo, <Update>: cập nhật trạng thái, <Draw>: Vẽ ra màn hình, <Cleanup> clear. 

## GameStateManager

- Đơn giản là nơi xử lí xem layer nào đang ở trên cùng và liên tục update layer đó và vẽ ra màn hình
- Có 4 thao tác chính là <PushState>: thêm layer, <PopState>: xóa layer trên cùng, <Update>: bắt layer trên cùng phại cập nhật trạnh thái, <Draw>: vẽ layer trên cùng ra màn hình.

## Luồng code
- bắt đầu từ hàm main.cpp (source\main.cpp)
...(hết)

## Tính năng đã có
- Thay đổi kích thước màn hình(resolution) trong mục setting
$ =================================================== $