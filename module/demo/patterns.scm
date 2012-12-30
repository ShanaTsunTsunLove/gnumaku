(define-module (demo patterns)
  #:use-module (oop goops)
  #:use-module (gnumaku generics)
  #:use-module (gnumaku core)
  #:use-module (gnumaku math)
  #:use-module (gnumaku bullet)
  #:use-module (gnumaku coroutine)
  #:use-module (gnumaku scene-graph)
  #:use-module (demo actor)
  #:use-module (demo level)
  #:export (spiral1 spiral2 polar-rose fire-at-player))

(define (clamp n min-n max-n)
  (max min-n (min max-n n)))

(define-coroutine (homing-bullet bullet target speed turn)
  (let* ((pos (bullet-position bullet))
         (target-pos (vector2-sub (position target) pos))
         (turn (if (> (vector2-cross pos target-pos) 0) turn (* -1 turn))))
    (set-bullet-movement bullet speed (+ (bullet-direction bullet) turn) 0 0))
  (bullet-wait bullet 1)
  (homing-bullet bullet target speed turn))

(define-coroutine (sine-wave bullet)
  (define angle-step 20)
  
  (define (step angle)
    (set-bullet-direction bullet (+ (bullet-direction bullet) (* 8 (sin-deg angle))))
    (bullet-wait bullet 1)
    (step (+ angle angle-step)))

  (step 0))

(define-coroutine (explode bullet system delay count speed angle-var)
  (define step (/ 360 count))
  
  (define (emit i)
    (let ((direction (+ (* i step) (random angle-var))))
      (emit-simple-bullet system (bullet-position bullet)
                          speed direction 'sword)))
  
  (bullet-wait bullet delay)
  (kill-bullet bullet)
  (repeat count emit))

(define-coroutine (spiral1 actor)
  (let ((step (/ 360 15)))
    (define (spiral angle)
      (let ((system (bullet-system actor))
            (player (player (level actor)))
            (direction (+ 90 (* 30 (sin-deg angle)))))
        (emit-script-bullet system (position actor) 'bright
                            (lambda (bullet)
                              (set-bullet-movement bullet 4 direction 0 0)
                              (set-bullet-color bullet
                                                (make-color-f (random:exp) (random:exp)
                                                              (random:exp) 1))
                              ;;(homing-bullet bullet player 3 1))))
                              (explode bullet system 45 6 4 30))))
                              ;; (sine-wave bullet))))
      (wait actor 6)
      (spiral (+ angle step)))

    (spiral 0)))

(define-coroutine (spiral2 actor)
  (let loop ((angle 0))
    (let ((scale (* 2 (random:exp))))
      (emit-bullet (bullet-system actor) (position actor) 2
                   (random (+ angle 20)) 'medium-blue
                   #:color (make-color-f (random:exp) (random:exp) (random:exp) 1)
                   #:scale (make-vector2 scale scale)))
    (wait actor 2)
    (loop (+ angle 20))))
